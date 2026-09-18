#include "face-mesh.hpp"

#include <onnxruntime_cxx_api.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr int MODEL_WIDTH = 192;
constexpr int MODEL_HEIGHT = 192;
constexpr int MODEL_CHANNELS = 3;
constexpr int LANDMARK_COUNT = 468;

struct face_mesh_onnx {
	Ort::Env env;
	Ort::SessionOptions session_options;
	std::unique_ptr<Ort::Session> session;

	std::string input_name;
	std::vector<std::string> output_names;

	face_mesh_onnx()
		: env(ORT_LOGGING_LEVEL_WARNING, "obs-face-mesh")
	{
		session_options.SetGraphOptimizationLevel(
			GraphOptimizationLevel::ORT_ENABLE_ALL);

		/*
		 * Keep inference reasonably lightweight for OBS.
		 * We can tune these later.
		 */
		session_options.SetIntraOpNumThreads(1);
		session_options.SetInterOpNumThreads(1);
	}
};

static inline uint8_t sample_channel(const uint8_t *data,
				     int width,
				     int height,
				     int stride,
				     int x,
				     int y,
				     int channel)
{
	x = std::clamp(x, 0, width - 1);
	y = std::clamp(y, 0, height - 1);

	/*
	 * OBS stagesurface is BGRA.
	 *
	 * channel:
	 *   0 = R
	 *   1 = G
	 *   2 = B
	 */
	const uint8_t *pixel = data + y * stride + x * 4;

	switch (channel) {
	case 0:
		return pixel[2]; /* R */
	case 1:
		return pixel[1]; /* G */
	default:
		return pixel[0]; /* B */
	}
}

} // namespace


struct face_mesh_tracker::impl {
	std::unique_ptr<face_mesh_onnx> onnx;
};


face_mesh_tracker::face_mesh_tracker()
	: pimpl(new impl)
{
}


face_mesh_tracker::~face_mesh_tracker()
{
	shutdown();
	delete pimpl;
	pimpl = nullptr;
}


bool face_mesh_tracker::initialize(const char *model_path)
{
	shutdown();

	if (!model_path || !*model_path)
		return false;

	try {
		pimpl->onnx = std::make_unique<face_mesh_onnx>();

#ifdef _WIN32
		/*
		 * ONNX Runtime expects wchar_t paths on Windows.
		 */
		int required =
			MultiByteToWideChar(CP_UTF8, 0, model_path, -1,
					    nullptr, 0);

		if (required <= 0) {
			pimpl->onnx.reset();
			return false;
		}

		std::wstring wide_path(
			static_cast<size_t>(required), L'\0');

		MultiByteToWideChar(
			CP_UTF8,
			0,
			model_path,
			-1,
			wide_path.data(),
			required);

		pimpl->onnx->session =
			std::make_unique<Ort::Session>(
				pimpl->onnx->env,
				wide_path.c_str(),
				pimpl->onnx->session_options);
#else
		pimpl->onnx->session =
			std::make_unique<Ort::Session>(
				pimpl->onnx->env,
				model_path,
				pimpl->onnx->session_options);
#endif

		Ort::AllocatorWithDefaultOptions allocator;

		/*
		 * Input name.
		 */
		{
			auto name =
				pimpl->onnx->session->GetInputNameAllocated(
					0, allocator);

			pimpl->onnx->input_name = name.get();
		}

		/*
		 * Save every output name because the model may expose
		 * both landmark and confidence/presence outputs.
		 */
		size_t output_count =
			pimpl->onnx->session->GetOutputCount();

		pimpl->onnx->output_names.clear();
		pimpl->onnx->output_names.reserve(output_count);

		for (size_t i = 0; i < output_count; ++i) {
			auto name =
				pimpl->onnx->session->GetOutputNameAllocated(
					i, allocator);

			pimpl->onnx->output_names.emplace_back(name.get());
		}

		points.clear();
		face_found = false;

		return true;

	} catch (const Ort::Exception &) {
		pimpl->onnx.reset();
		points.clear();
		face_found = false;
		return false;
	}
}


void face_mesh_tracker::shutdown()
{
	if (pimpl)
		pimpl->onnx.reset();

	points.clear();
	face_found = false;
}


bool face_mesh_tracker::process_frame(const uint8_t *data,
				      int width,
				      int height,
				      int stride,
				      int64_t timestamp_ms)
{
	(void)timestamp_ms;

	if (!data ||
	    width <= 0 ||
	    height <= 0 ||
	    stride <= 0 ||
	    !pimpl ||
	    !pimpl->onnx ||
	    !pimpl->onnx->session) {
		face_found = false;
		points.clear();
		return false;
	}

	/*
	 * TEMPORARY preprocessing:
	 *
	 * Use a centered square crop.
	 *
	 * Once inference is verified, this will be replaced with
	 * the rectangle supplied by the existing face tracker.
	 */
	const int crop_size = std::min(width, height);
	const int crop_x = (width - crop_size) / 2;
	const int crop_y = (height - crop_size) / 2;

	/*
	 * Model input:
	 *
	 * NCHW
	 * [1, 3, 192, 192]
	 *
	 * RGB float32, normalized to 0..1.
	 */
	std::vector<float> input(
		MODEL_CHANNELS *
		MODEL_WIDTH *
		MODEL_HEIGHT);

	for (int y = 0; y < MODEL_HEIGHT; ++y) {
		const float src_y =
			crop_y +
			((y + 0.5f) / MODEL_HEIGHT) *
				crop_size;

		const int iy =
			std::clamp(
				static_cast<int>(src_y),
				0,
				height - 1);

		for (int x = 0; x < MODEL_WIDTH; ++x) {
			const float src_x =
				crop_x +
				((x + 0.5f) / MODEL_WIDTH) *
					crop_size;

			const int ix =
				std::clamp(
					static_cast<int>(src_x),
					0,
					width - 1);

			for (int c = 0; c < 3; ++c) {
				const size_t index =
					static_cast<size_t>(c) *
						MODEL_WIDTH *
						MODEL_HEIGHT +
					static_cast<size_t>(y) *
						MODEL_WIDTH +
					x;

				input[index] =
					static_cast<float>(
						sample_channel(
							data,
							width,
							height,
							stride,
							ix,
							iy,
							c)) /
					255.0f;
			}
		}
	}

	std::array<int64_t, 4> input_shape = {
		1,
		MODEL_CHANNELS,
		MODEL_HEIGHT,
		MODEL_WIDTH
	};

	Ort::MemoryInfo memory_info =
		Ort::MemoryInfo::CreateCpu(
			OrtArenaAllocator,
			OrtMemTypeDefault);

	Ort::Value input_tensor =
		Ort::Value::CreateTensor<float>(
			memory_info,
			input.data(),
			input.size(),
			input_shape.data(),
			input_shape.size());

	const char *input_names[] = {
		pimpl->onnx->input_name.c_str()
	};

	std::vector<const char *> output_name_ptrs;
	output_name_ptrs.reserve(
		pimpl->onnx->output_names.size());

	for (const auto &name :
	     pimpl->onnx->output_names) {
		output_name_ptrs.push_back(name.c_str());
	}

	try {
		auto outputs =
			pimpl->onnx->session->Run(
				Ort::RunOptions{nullptr},
				input_names,
				&input_tensor,
				1,
				output_name_ptrs.data(),
				output_name_ptrs.size());

		/*
		 * Find an output containing at least
		 * 468 * 3 floats.
		 */
		Ort::Value *landmark_output = nullptr;

		for (auto &output : outputs) {
			if (!output.IsTensor())
				continue;

			auto info =
				output.GetTensorTypeAndShapeInfo();

			if (info.GetElementType() !=
			    ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
				continue;

			const size_t count =
				info.GetElementCount();

			if (count >=
			    static_cast<size_t>(
				    LANDMARK_COUNT * 3)) {
				landmark_output = &output;
				break;
			}
		}

		if (!landmark_output) {
			face_found = false;
			points.clear();
			return false;
		}

		const float *landmarks =
			landmark_output->
				GetTensorData<float>();

		points.resize(LANDMARK_COUNT);

		for (int i = 0;
		     i < LANDMARK_COUNT;
		     ++i) {

			float lx =
				landmarks[i * 3 + 0];

			float ly =
				landmarks[i * 3 + 1];

			float lz =
				landmarks[i * 3 + 2];

			/*
			 * This model reports landmark x/y in
			 * 192x192 model coordinates.
			 *
			 * Convert them to normalized coordinates
			 * within our original OBS frame because
			 * draw_face_mesh() expects 0..1.
			 */
			const float nx =
				(crop_x +
				 (lx / MODEL_WIDTH) *
					 crop_size) /
				static_cast<float>(width);

			const float ny =
				(crop_y +
				 (ly / MODEL_HEIGHT) *
					 crop_size) /
				static_cast<float>(height);

			points[i].x = nx;
			points[i].y = ny;

			/*
			 * Z is retained in approximately model-space
			 * units. Rendering currently only uses X/Y.
			 */
			points[i].z = lz / MODEL_WIDTH;
		}

		face_found = true;
		return true;

	} catch (const Ort::Exception &) {
		face_found = false;
		points.clear();
		return false;
	}
}

bool face_mesh_tracker::has_face() const
{
	return face_found;
}

const std::vector<face_mesh_point> &face_mesh_tracker::landmarks() const
{
	return points;
}