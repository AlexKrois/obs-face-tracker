#pragma once

#include <cstddef>

struct face_mesh_connection {
	int a;
	int b;
};

extern const face_mesh_connection FACE_MESH_OVAL[];
extern const size_t FACE_MESH_OVAL_COUNT;

extern const face_mesh_connection FACE_MESH_LEFT_EYE[];
extern const size_t FACE_MESH_LEFT_EYE_COUNT;

extern const face_mesh_connection FACE_MESH_RIGHT_EYE[];
extern const size_t FACE_MESH_RIGHT_EYE_COUNT;

extern const face_mesh_connection FACE_MESH_LEFT_EYEBROW[];
extern const size_t FACE_MESH_LEFT_EYEBROW_COUNT;

extern const face_mesh_connection FACE_MESH_RIGHT_EYEBROW[];
extern const size_t FACE_MESH_RIGHT_EYEBROW_COUNT;

extern const face_mesh_connection FACE_MESH_NOSE[];
extern const size_t FACE_MESH_NOSE_COUNT;

extern const face_mesh_connection FACE_MESH_LIPS[];
extern const size_t FACE_MESH_LIPS_COUNT;