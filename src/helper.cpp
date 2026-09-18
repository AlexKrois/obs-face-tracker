#include <obs-module.h>
#include "plugin-macros.generated.h"
#include "helper.hpp"

#include <cmath>

/*
 * ============================================================
 * DEBUG DRAWING SETTINGS
 * ============================================================
 *
 * DEBUG_RECT_THICKNESS:
 *   Blue face-detection / green tracking rectangles.
 *
 * DEBUG_LANDMARK_THICKNESS:
 *   Facial landmark line thickness.
 *
 * LANDMARK_SMOOTHING:
 *   Temporal smoothing applied ONLY to the displayed landmark
 *   lines. It does not modify the landmark coordinates used
 *   internally by the face tracker.
 *
 *   Lower = smoother, but more visual delay
 *   Higher = more responsive, but more jitter
 *
 *   Suggested:
 *     0.10 = very smooth
 *     0.15 = smooth
 *     0.20 = balanced
 *     0.30 = responsive
 *     1.00 = no smoothing
 */

static constexpr float DEBUG_RECT_THICKNESS = 0.01f;
static constexpr float DEBUG_LANDMARK_THICKNESS = 5.1f;
static constexpr float LANDMARK_SMOOTHING = 0.20f;


/*
 * Draw one thick line segment as a rectangle made from
 * a triangle strip.
 *
 * OBS/graphics GS_LINES does not provide the line thickness
 * we want, so actual geometry is generated instead.
 */
static void draw_thick_line(float x0, float y0,
			    float x1, float y1,
			    float thickness)
{
	const float dx = x1 - x0;
	const float dy = y1 - y0;

	const float length = std::sqrt(dx * dx + dy * dy);

	if (length <= 0.0001f)
		return;

	/*
	 * Calculate a perpendicular vector whose length is
	 * half of the desired line thickness.
	 */
	const float half = thickness * 0.5f;

	const float px = (-dy / length) * half;
	const float py = (dx / length) * half;

	gs_render_start(false);

	/*
	 * Triangle strip:
	 *
	 *   A----------------B
	 *   |                |
	 *   |                |
	 *   C----------------D
	 */
	gs_vertex2f(x0 + px, y0 + py);
	gs_vertex2f(x1 + px, y1 + py);
	gs_vertex2f(x0 - px, y0 - py);
	gs_vertex2f(x1 - px, y1 - py);

	gs_render_stop(GS_TRISTRIP);
}


/*
 * Draw a rectangle using four thick line segments.
 */
static void draw_thick_rect(rect_s r, float thickness)
{
	if (r.x0 >= r.x1 || r.y0 >= r.y1)
		return;

	const float x0 = (float)r.x0;
	const float y0 = (float)r.y0;
	const float x1 = (float)r.x1;
	const float y1 = (float)r.y1;

	/* Top */
	draw_thick_line(x0, y0, x1, y0, thickness);

	/* Right */
	draw_thick_line(x1, y0, x1, y1, thickness);

	/* Bottom */
	draw_thick_line(x1, y1, x0, y1, thickness);

	/* Left */
	draw_thick_line(x0, y1, x0, y0, thickness);
}


/*
 * Draw face detection / tracking rectangle.
 *
 * This replaces the original GS_LINES rendering with thick
 * geometry while preserving the original upsize behaviour.
 */
void draw_rect_upsize(rect_s r,
		      float upsize_l,
		      float upsize_r,
		      float upsize_t,
		      float upsize_b)
{
	if (r.x0 >= r.x1 || r.y0 >= r.y1)
		return;

	int w = r.x1 - r.x0;
	int h = r.y1 - r.y0;

	float dx0 = w * upsize_l;
	float dx1 = w * upsize_r;
	float dy0 = h * upsize_t;
	float dy1 = h * upsize_b;

	/*
	 * The original plugin draws the original rectangle too
	 * when an upsize adjustment is active.
	 */
	if (std::abs(dx0) >= 0.5f ||
	    std::abs(dy1) >= 0.5f ||
	    std::abs(dx1) >= 0.5f ||
	    std::abs(dy0) >= 0.5f) {
		draw_thick_rect(r, DEBUG_RECT_THICKNESS);
	}

	r.x0 -= (int)dx0;
	r.x1 += (int)dx1;
	r.y0 -= (int)dy0;
	r.y1 += (int)dy1;

	draw_thick_rect(r, DEBUG_RECT_THICKNESS);
}


/*
 * Original landmark area calculation.
 */
float landmark_area(const std::vector<pointf_s> &landmark)
{
	// TODO: implement area calculation for other models
	// Maybe, use the area of the maximum convex polygon.

	float ret = 0.0f;

	const static int ii5[] = {
		1, // center
		// 0, 4, 2, 3, 1,
		0, 1, 3, 2, 4, 0, -1
	};

	const static int ii68[] = {
		30, // center
		0,  1,  2,  3,  4,  5,  6,  7,
		8,  9,  10, 11, 12, 13, 14, 15,
		16, 26, 25, 24, 23, 22, 21, 20,
		19, 18, 17, 0, -1
	};

	const int *ii =
		landmark.size() == 68 ? ii68 :
		landmark.size() == 5  ? ii5  :
		NULL;

	if (!ii)
		return 0.0f;

	pointf_s c = landmark[ii[0]];

	for (int i = 1; ii[i + 1] >= 0; i++) {
		float x1 = landmark[ii[i]].x - c.x;
		float y1 = landmark[ii[i]].y - c.y;

		float x2 = landmark[ii[i + 1]].x - c.x;
		float y2 = landmark[ii[i + 1]].y - c.y;

		ret += (x2 * y1 - x1 * y2) * 0.5f;
	}

	return ret;
}


/*
 * Original landmark center calculation.
 */
pointf_s landmark_center(const std::vector<pointf_s> &landmark)
{
	pointf_s ret = {0.0f, 0.0f};

	for (size_t i = 0; i < landmark.size(); i++) {
		ret.x += landmark[i].x;
		ret.y += landmark[i].y;
	}

	ret.x /= landmark.size();
	ret.y /= landmark.size();

	return ret;
}


/*
 * Draw facial landmarks with configurable thickness and
 * display-only temporal smoothing.
 *
 * Supports:
 *
 *   5-point landmark model
 *   68-point landmark model
 *
 * The raw landmark vector is NOT modified.
 */
void draw_landmark(const std::vector<pointf_s> &landmark)
{
	if (landmark.size() != 5 && landmark.size() != 68)
		return;

	/*
	 * Previous displayed landmark coordinates.
	 *
	 * On the first frame these are initialized directly from
	 * the detector. Subsequent frames are interpolated toward
	 * the new detector coordinates.
	 */
	static std::vector<pointf_s> smoothed_landmark;

	/*
	 * Initialize on the first frame, or reset if the landmark
	 * model changes between 5-point and 68-point.
	 */
	if (smoothed_landmark.size() != landmark.size()) {
		smoothed_landmark = landmark;
	} else {
		for (size_t i = 0; i < landmark.size(); i++) {
			smoothed_landmark[i].x +=
				(landmark[i].x -
				 smoothed_landmark[i].x) *
				LANDMARK_SMOOTHING;

			smoothed_landmark[i].y +=
				(landmark[i].y -
				 smoothed_landmark[i].y) *
				LANDMARK_SMOOTHING;
		}
	}

	/*
	 * Convenience function for connecting two SMOOTHED
	 * landmark points.
	 */
	auto line = [&](size_t a, size_t b) {
		draw_thick_line(
			smoothed_landmark[a].x,
			smoothed_landmark[a].y,
			smoothed_landmark[b].x,
			smoothed_landmark[b].y,
			DEBUG_LANDMARK_THICKNESS
		);
	};


	/*
	 * ========================================================
	 * 5-POINT LANDMARK MODEL
	 * ========================================================
	 *
	 * Connection order:
	 *
	 * 0 -> 1 -> 3 -> 2 -> 4 -> 0
	 */
	if (smoothed_landmark.size() == 5) {
		line(0, 1);
		line(1, 3);
		line(3, 2);
		line(2, 4);
		line(4, 0);

		return;
	}


	/*
	 * ========================================================
	 * 68-POINT LANDMARK MODEL
	 * ========================================================
	 */


	/*
	 * Jaw
	 *
	 * 0 -> 1 -> ... -> 16
	 */
	for (size_t i = 0; i < 16; i++)
		line(i, i + 1);


	/*
	 * Eyebrow 1
	 *
	 * 17 -> ... -> 21
	 */
	for (size_t i = 17; i < 21; i++)
		line(i, i + 1);


	/*
	 * Eyebrow 2
	 *
	 * 22 -> ... -> 26
	 */
	for (size_t i = 22; i < 26; i++)
		line(i, i + 1);


	/*
	 * Nose bridge
	 *
	 * 27 -> ... -> 30
	 */
	for (size_t i = 27; i < 30; i++)
		line(i, i + 1);


	/*
	 * Bottom of nose
	 *
	 * 31 -> ... -> 35
	 */
	for (size_t i = 31; i < 35; i++)
		line(i, i + 1);


	/*
	 * Eye 1
	 *
	 * 36 -> ... -> 41 -> 36
	 */
	for (size_t i = 36; i < 41; i++)
		line(i, i + 1);

	line(41, 36);


	/*
	 * Eye 2
	 *
	 * 42 -> ... -> 47 -> 42
	 */
	for (size_t i = 42; i < 47; i++)
		line(i, i + 1);

	line(47, 42);


	/*
	 * Outer mouth
	 *
	 * 48 -> ... -> 59 -> 48
	 */
	for (size_t i = 48; i < 59; i++)
		line(i, i + 1);

	line(59, 48);


	/*
	 * Inner mouth
	 *
	 * 60 -> ... -> 67 -> 60
	 */
	for (size_t i = 60; i < 67; i++)
		line(i, i + 1);

	line(67, 60);
}


/*
 * Original debug data file handling.
 */
void debug_data_open(FILE **dest,
		     char **last_name,
		     obs_data_t *settings,
		     const char *name)
{
	const char *debug_data =
		obs_data_get_string(settings, name);

	/*
	 * If the file name is not changed, just return.
	 */
	if (*last_name &&
	    debug_data &&
	    strcmp(*last_name, debug_data) == 0)
		return;

	/*
	 * If both file names are empty, just return.
	 */
	if (!*last_name &&
	    (!debug_data || !*debug_data))
		return;

	if (*dest)
		fclose(*dest);

	*dest = NULL;

	if (*last_name)
		bfree(*last_name);

	*last_name = NULL;

	if (debug_data && *debug_data) {
		*dest = fopen(debug_data, "a");

		if (!*dest) {
			blog(
				LOG_ERROR,
				"%s: Failed to open file \"%s\"",
				name,
				debug_data
			);
		}

		*last_name = bstrdup(debug_data);
	}
}
