#include <obs-module.h>
#include "plugin-macros.generated.h"
#include "helper.hpp"
#include "face-mesh.hpp"
#include "face-mesh-connections.hpp"

#include <cmath>

static constexpr float DEBUG_RECT_THICKNESS = 0.01f;

static void draw_thick_line(const pointf_s &a, const pointf_s &b, float thickness)
{
	const float dx = b.x - a.x;
	const float dy = b.y - a.y;
	const float length = std::sqrt(dx * dx + dy * dy);

	if (length <= 0.0001f)
		return;

	const float half = thickness * 0.5f;
	const float px = (-dy / length) * half;
	const float py = (dx / length) * half;

	gs_render_start(false);
	gs_vertex2f(a.x + px, a.y + py);
	gs_vertex2f(a.x - px, a.y - py);
	gs_vertex2f(b.x + px, b.y + py);
	gs_vertex2f(b.x - px, b.y - py);
	gs_render_stop(GS_TRISTRIP);
}

static void draw_thick_rect(float x0, float y0, float x1, float y1, float thickness)
{
	pointf_s p0{x0, y0};
	pointf_s p1{x0, y1};
	pointf_s p2{x1, y1};
	pointf_s p3{x1, y0};

	draw_thick_line(p0, p1, thickness);
	draw_thick_line(p1, p2, thickness);
	draw_thick_line(p2, p3, thickness);
	draw_thick_line(p3, p0, thickness);
}

static void draw_mesh_connections(
	const std::vector<pointf_s> &points,
	const face_mesh_connection *connections,
	size_t count,
	float thickness)
{
	for (size_t i = 0; i < count; i++) {
		const int a = connections[i].a;
		const int b = connections[i].b;

		if (a < 0 || b < 0)
			continue;

		if ((size_t)a >= points.size() ||
		    (size_t)b >= points.size())
			continue;

		draw_thick_line(points[a], points[b], thickness);
	}
}

void draw_face_mesh(
	const std::vector<face_mesh_point> &landmarks,
	float width,
	float height,
	float smoothing,
	float thickness)
{
	if (landmarks.empty())
		return;

	if (smoothing < 0.0f)
		smoothing = 0.0f;

	if (smoothing > 0.95f)
		smoothing = 0.95f;

	if (thickness < 0.5f)
		thickness = 0.5f;

	if (thickness > 20.0f)
		thickness = 20.0f;

	const float alpha = 1.0f - smoothing;

	static std::vector<pointf_s> smoothed;

	if (smoothed.size() != landmarks.size()) {
		smoothed.resize(landmarks.size());

		for (size_t i = 0; i < landmarks.size(); i++) {
			smoothed[i].x = landmarks[i].x * width;
			smoothed[i].y = landmarks[i].y * height;
		}
	} else {
		for (size_t i = 0; i < landmarks.size(); i++) {

			const float x =
				landmarks[i].x * width;

			const float y =
				landmarks[i].y * height;

			smoothed[i].x +=
				(x - smoothed[i].x) * alpha;

			smoothed[i].y +=
				(y - smoothed[i].y) * alpha;
		}
	}

	draw_mesh_connections(
		smoothed,
		FACE_MESH_OVAL,
		FACE_MESH_OVAL_COUNT,
		thickness);

	draw_mesh_connections(
		smoothed,
		FACE_MESH_LEFT_EYE,
		FACE_MESH_LEFT_EYE_COUNT,
		thickness);

	draw_mesh_connections(
		smoothed,
		FACE_MESH_RIGHT_EYE,
		FACE_MESH_RIGHT_EYE_COUNT,
		thickness);

	draw_mesh_connections(
		smoothed,
		FACE_MESH_LEFT_EYEBROW,
		FACE_MESH_LEFT_EYEBROW_COUNT,
		thickness);

	draw_mesh_connections(
		smoothed,
		FACE_MESH_RIGHT_EYEBROW,
		FACE_MESH_RIGHT_EYEBROW_COUNT,
		thickness);

	draw_mesh_connections(
		smoothed,
		FACE_MESH_NOSE,
		FACE_MESH_NOSE_COUNT,
		thickness);

	draw_mesh_connections(
		smoothed,
		FACE_MESH_LIPS,
		FACE_MESH_LIPS_COUNT,
		thickness);
}

void draw_rect_upsize(rect_s r, float upsize_l, float upsize_r, float upsize_t, float upsize_b)
{
	const float w = (float)(r.x1 - r.x0);
	const float h = (float)(r.y1 - r.y0);

	const float x0 = (float)r.x0 - w * upsize_l;
	const float x1 = (float)r.x1 + w * upsize_r;
	const float y0 = (float)r.y0 - h * upsize_t;
	const float y1 = (float)r.y1 + h * upsize_b;

	draw_thick_rect(x0, y0, x1, y1, DEBUG_RECT_THICKNESS);
}

float landmark_area(const std::vector<pointf_s> &landmark)
{
	if (landmark.empty())
		return 0.0f;

	float x0 = landmark[0].x;
	float x1 = landmark[0].x;
	float y0 = landmark[0].y;
	float y1 = landmark[0].y;

	for (const auto &p : landmark) {
		x0 = std::min(x0, p.x);
		x1 = std::max(x1, p.x);
		y0 = std::min(y0, p.y);
		y1 = std::max(y1, p.y);
	}

	return (x1 - x0) * (y1 - y0);
}

pointf_s landmark_center(const std::vector<pointf_s> &landmark)
{
	pointf_s center{0.0f, 0.0f};

	if (landmark.empty())
		return center;

	for (const auto &p : landmark) {
		center.x += p.x;
		center.y += p.y;
	}

	center.x /= (float)landmark.size();
	center.y /= (float)landmark.size();
	return center;
}

void draw_landmark(const std::vector<pointf_s> &landmark, float smoothing, float thickness)
{
	if (landmark.size() != 5 && landmark.size() != 68)
		return;

	if (smoothing < 0.0f)
		smoothing = 0.0f;
	if (smoothing > 0.95f)
		smoothing = 0.95f;

	if (thickness < 0.5f)
		thickness = 0.5f;
	if (thickness > 20.0f)
		thickness = 20.0f;

	const float alpha = 1.0f - smoothing;

	static std::vector<pointf_s> smoothed_landmark;

	if (smoothed_landmark.size() != landmark.size()) {
		smoothed_landmark = landmark;
	} else {
		for (size_t i = 0; i < landmark.size(); i++) {
			smoothed_landmark[i].x +=
				(landmark[i].x - smoothed_landmark[i].x) * alpha;
			smoothed_landmark[i].y +=
				(landmark[i].y - smoothed_landmark[i].y) * alpha;
		}
	}

	auto line = [&](int a, int b) {
		draw_thick_line(smoothed_landmark[a], smoothed_landmark[b], thickness);
	};

	if (smoothed_landmark.size() == 5) {
		line(0, 1);
		line(1, 3);
		line(3, 2);
		line(2, 4);
		line(4, 0);
		return;
	}

	// Jaw: 0-16
	for (int i = 0; i < 16; i++)
		line(i, i + 1);

	// Eyebrows: 17-21 and 22-26
	for (int i = 17; i < 21; i++)
		line(i, i + 1);
	for (int i = 22; i < 26; i++)
		line(i, i + 1);

	// Nose bridge: 27-30
	for (int i = 27; i < 30; i++)
		line(i, i + 1);

	// Bottom of nose: 31-35
	for (int i = 31; i < 35; i++)
		line(i, i + 1);

	// Left eye: 36-41, closed
	for (int i = 36; i < 41; i++)
		line(i, i + 1);
	line(41, 36);

	// Right eye: 42-47, closed
	for (int i = 42; i < 47; i++)
		line(i, i + 1);
	line(47, 42);

	// Outer mouth: 48-59, closed
	for (int i = 48; i < 59; i++)
		line(i, i + 1);
	line(59, 48);

	// Inner mouth: 60-67, closed
	for (int i = 60; i < 67; i++)
		line(i, i + 1);
	line(67, 60);
}

void debug_data_open(FILE **dest, char **last_name, obs_data_t *settings, const char *name)
{
#ifdef ENABLE_DEBUG_DATA
	const char *filename = obs_data_get_string(settings, name);

	if (!filename || !*filename) {
		if (*dest) {
			fclose(*dest);
			*dest = NULL;
		}
		bfree(*last_name);
		*last_name = NULL;
		return;
	}

	if (*last_name && strcmp(*last_name, filename) == 0)
		return;

	if (*dest) {
		fclose(*dest);
		*dest = NULL;
	}

	bfree(*last_name);
	*last_name = bstrdup(filename);
	*dest = fopen(filename, "a");
#else
	UNUSED_PARAMETER(dest);
	UNUSED_PARAMETER(last_name);
	UNUSED_PARAMETER(settings);
	UNUSED_PARAMETER(name);
#endif
}
