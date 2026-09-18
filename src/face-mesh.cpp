#include "face-mesh.hpp"

face_mesh_tracker::face_mesh_tracker()
{
}

face_mesh_tracker::~face_mesh_tracker()
{
	shutdown();
}

bool face_mesh_tracker::initialize(const char *model_path)
{
	(void)model_path;

	points.clear();
	face_found = false;

	/*
	 * Model initialization goes here.
	 */

	return true;
}

void face_mesh_tracker::shutdown()
{
	points.clear();
	face_found = false;
}

bool face_mesh_tracker::process_frame(const uint8_t *data,
				      int width,
				      int height,
				      int stride,
				      int64_t timestamp_ms)
{
	(void)data;
	(void)width;
	(void)height;
	(void)stride;
	(void)timestamp_ms;

	/*
	 * Face-landmark inference goes here.
	 *
	 * Fill:
	 *
	 * points[i].x
	 * points[i].y
	 * points[i].z
	 *
	 * using normalized coordinates 0..1.
	 */

	return false;
}

bool face_mesh_tracker::has_face() const
{
	return face_found;
}

const std::vector<face_mesh_point> &face_mesh_tracker::landmarks() const
{
	return points;
}