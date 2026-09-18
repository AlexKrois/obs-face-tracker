void draw_landmark(const std::vector<pointf_s> &landmark)
{
	if (landmark.size() != 5 && landmark.size() != 68)
		return;

	/*
	 * Landmark display smoothing.
	 *
	 * 0.10 = very smooth, more delay
	 * 0.20 = smooth
	 * 0.30 = good balance
	 * 0.50 = responsive
	 * 1.00 = no smoothing
	 */
	static constexpr float LANDMARK_SMOOTHING = 0.20f;

	/*
	 * Previous displayed landmark positions.
	 *
	 * These affect only the visualization. The actual face tracker
	 * continues to use the original unsmoothed landmark data.
	 */
	static std::vector<pointf_s> smoothed_landmark;
	static bool initialized = false;

	/*
	 * Reinitialize if the landmark model changes between
	 * 5-point and 68-point.
	 */
	if (!initialized || smoothed_landmark.size() != landmark.size()) {
		smoothed_landmark = landmark;
		initialized = true;
	} else {
		for (size_t i = 0; i < landmark.size(); i++) {
			smoothed_landmark[i].x +=
				(landmark[i].x - smoothed_landmark[i].x) *
				LANDMARK_SMOOTHING;

			smoothed_landmark[i].y +=
				(landmark[i].y - smoothed_landmark[i].y) *
				LANDMARK_SMOOTHING;
		}
	}

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
	 * 5-point landmark model
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
	 * 68-point landmark model
	 */

	// Jaw: 0-16
	for (size_t i = 0; i < 16; i++)
		line(i, i + 1);

	// Right eyebrow: 17-21
	for (size_t i = 17; i < 21; i++)
		line(i, i + 1);

	// Left eyebrow: 22-26
	for (size_t i = 22; i < 26; i++)
		line(i, i + 1);

	// Nose bridge: 27-30
	for (size_t i = 27; i < 30; i++)
		line(i, i + 1);

	// Nose bottom: 31-35
	for (size_t i = 31; i < 35; i++)
		line(i, i + 1);

	// Right eye: 36-41
	for (size_t i = 36; i < 41; i++)
		line(i, i + 1);
	line(41, 36);

	// Left eye: 42-47
	for (size_t i = 42; i < 47; i++)
		line(i, i + 1);
	line(47, 42);

	// Outer mouth: 48-59
	for (size_t i = 48; i < 59; i++)
		line(i, i + 1);
	line(59, 48);

	// Inner mouth: 60-67
	for (size_t i = 60; i < 67; i++)
		line(i, i + 1);
	line(67, 60);
}
