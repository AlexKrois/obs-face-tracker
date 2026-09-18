#pragma once

#include <cstdint>
#include <vector>

struct face_mesh_point {
	float x;
	float y;
	float z;
};

class face_mesh_tracker {
public:
	face_mesh_tracker();
	~face_mesh_tracker();

	face_mesh_tracker(const face_mesh_tracker &) = delete;
	face_mesh_tracker &
	operator=(const face_mesh_tracker &) = delete;

	bool initialize(const char *model_path);
	void shutdown();

	bool process_frame(
		const uint8_t *data,
		int width,
		int height,
		int stride,
		int64_t timestamp_ms);

	bool has_face() const;

	const std::vector<face_mesh_point> &
	landmarks() const;

private:
	struct impl;
	impl *pimpl;

	std::vector<face_mesh_point> points;
	bool face_found = false;
};