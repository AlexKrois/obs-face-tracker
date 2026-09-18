#pragma once

#include <cstddef>

struct face_mesh_connection {
	int a;
	int b;
};

/*
 * MediaPipe-style landmark connections.
 *
 * These are the feature contours we want to render rather than
 * the complete triangulated face mesh.
 */

inline constexpr face_mesh_connection FACE_MESH_OVAL[] = {
	{10, 338},
	{338, 297},
	{297, 332},
	{332, 284},
	{284, 251},
	{251, 389},
	{389, 356},
	{356, 454},
	{454, 323},
	{323, 361},
	{361, 288},
	{288, 397},
	{397, 365},
	{365, 379},
	{379, 378},
	{378, 400},
	{400, 377},
	{377, 152},
	{152, 148},
	{148, 176},
	{176, 149},
	{149, 150},
	{150, 136},
	{136, 172},
	{172, 58},
	{58, 132},
	{132, 93},
	{93, 234},
	{234, 127},
	{127, 162},
	{162, 21},
	{21, 54},
	{54, 103},
	{103, 67},
	{67, 109},
	{109, 10}
};

inline constexpr face_mesh_connection FACE_MESH_LEFT_EYE[] = {
	{263, 249},
	{249, 390},
	{390, 373},
	{373, 374},
	{374, 380},
	{380, 381},
	{381, 382},
	{382, 362},
	{362, 398},
	{398, 384},
	{384, 385},
	{385, 386},
	{386, 387},
	{387, 388},
	{388, 466},
	{466, 263}
};

inline constexpr face_mesh_connection FACE_MESH_RIGHT_EYE[] = {
	{33, 7},
	{7, 163},
	{163, 144},
	{144, 145},
	{145, 153},
	{153, 154},
	{154, 155},
	{155, 133},
	{133, 173},
	{173, 157},
	{157, 158},
	{158, 159},
	{159, 160},
	{160, 161},
	{161, 246},
	{246, 33}
};

inline constexpr face_mesh_connection FACE_MESH_LEFT_EYEBROW[] = {
	{276, 283},
	{283, 282},
	{282, 295},
	{295, 285},
	{300, 293},
	{293, 334},
	{334, 296},
	{296, 336}
};

inline constexpr face_mesh_connection FACE_MESH_RIGHT_EYEBROW[] = {
	{46, 53},
	{53, 52},
	{52, 65},
	{65, 55},
	{70, 63},
	{63, 105},
	{105, 66},
	{66, 107}
};

inline constexpr face_mesh_connection FACE_MESH_NOSE[] = {
	{168, 6},
	{6, 197},
	{197, 195},
	{195, 5},
	{5, 4},
	{4, 1},
	{1, 19},
	{19, 94},
	{94, 2},

	{98, 97},
	{97, 2},
	{2, 326},
	{326, 327}
};

inline constexpr face_mesh_connection FACE_MESH_LIPS[] = {
	/* outer lip */
	{61, 146},
	{146, 91},
	{91, 181},
	{181, 84},
	{84, 17},
	{17, 314},
	{314, 405},
	{405, 321},
	{321, 375},
	{375, 291},
	{291, 409},
	{409, 270},
	{270, 269},
	{269, 267},
	{267, 0},
	{0, 37},
	{37, 39},
	{39, 40},
	{40, 185},
	{185, 61},

	/* inner lip */
	{78, 95},
	{95, 88},
	{88, 178},
	{178, 87},
	{87, 14},
	{14, 317},
	{317, 402},
	{402, 318},
	{318, 324},
	{324, 308},
	{308, 415},
	{415, 310},
	{310, 311},
	{311, 312},
	{312, 13},
	{13, 82},
	{82, 81},
	{81, 80},
	{80, 191},
	{191, 78}
};

/*
 * Additional structural contours.
 */

inline constexpr face_mesh_connection FACE_MESH_CENTER[] = {
	{10, 151},
	{151, 9},
	{9, 8},
	{8, 168},
	{168, 6},
	{6, 197},
	{197, 195},
	{195, 5},
	{5, 4},
	{4, 1},
};

inline constexpr size_t FACE_MESH_CENTER_COUNT =
	sizeof(FACE_MESH_CENTER) /
	sizeof(FACE_MESH_CENTER[0]);


inline constexpr face_mesh_connection FACE_MESH_LEFT_CHEEK[] = {
	{234, 93},
	{93, 132},
	{132, 58},
	{58, 172},
	{172, 136},
	{136, 150},

	{127, 123},
	{123, 50},
	{50, 101},
	{101, 205},
	{205, 36},
};

inline constexpr size_t FACE_MESH_LEFT_CHEEK_COUNT =
	sizeof(FACE_MESH_LEFT_CHEEK) /
	sizeof(FACE_MESH_LEFT_CHEEK[0]);


inline constexpr face_mesh_connection FACE_MESH_RIGHT_CHEEK[] = {
	{454, 323},
	{323, 361},
	{361, 288},
	{288, 397},
	{397, 365},
	{365, 379},

	{356, 352},
	{352, 280},
	{280, 330},
	{330, 425},
	{425, 266},
};

inline constexpr size_t FACE_MESH_RIGHT_CHEEK_COUNT =
	sizeof(FACE_MESH_RIGHT_CHEEK) /
	sizeof(FACE_MESH_RIGHT_CHEEK[0]);


inline constexpr face_mesh_connection FACE_MESH_LEFT_MIDFACE[] = {
	{33, 130},
	{130, 117},
	{117, 118},
	{118, 119},
	{119, 120},
	{120, 121},
	{121, 128},
};

inline constexpr size_t FACE_MESH_LEFT_MIDFACE_COUNT =
	sizeof(FACE_MESH_LEFT_MIDFACE) /
	sizeof(FACE_MESH_LEFT_MIDFACE[0]);


inline constexpr face_mesh_connection FACE_MESH_RIGHT_MIDFACE[] = {
	{263, 359},
	{359, 346},
	{346, 347},
	{347, 348},
	{348, 349},
	{349, 350},
	{350, 357},
};

inline constexpr size_t FACE_MESH_RIGHT_MIDFACE_COUNT =
	sizeof(FACE_MESH_RIGHT_MIDFACE) /
	sizeof(FACE_MESH_RIGHT_MIDFACE[0]);

inline constexpr size_t FACE_MESH_OVAL_COUNT =
	sizeof(FACE_MESH_OVAL) / sizeof(FACE_MESH_OVAL[0]);

inline constexpr size_t FACE_MESH_LEFT_EYE_COUNT =
	sizeof(FACE_MESH_LEFT_EYE) / sizeof(FACE_MESH_LEFT_EYE[0]);

inline constexpr size_t FACE_MESH_RIGHT_EYE_COUNT =
	sizeof(FACE_MESH_RIGHT_EYE) / sizeof(FACE_MESH_RIGHT_EYE[0]);

inline constexpr size_t FACE_MESH_LEFT_EYEBROW_COUNT =
	sizeof(FACE_MESH_LEFT_EYEBROW) / sizeof(FACE_MESH_LEFT_EYEBROW[0]);

inline constexpr size_t FACE_MESH_RIGHT_EYEBROW_COUNT =
	sizeof(FACE_MESH_RIGHT_EYEBROW) / sizeof(FACE_MESH_RIGHT_EYEBROW[0]);

inline constexpr size_t FACE_MESH_NOSE_COUNT =
	sizeof(FACE_MESH_NOSE) / sizeof(FACE_MESH_NOSE[0]);

inline constexpr size_t FACE_MESH_LIPS_COUNT =
	sizeof(FACE_MESH_LIPS) / sizeof(FACE_MESH_LIPS[0]);