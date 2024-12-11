#ifndef SCENE_SETTINGS_H
#define SCENE_SETTINGS_H



namespace mnemosy::graphics {

	struct SceneSettings {
	public:
		float globalExposure = 0.0f;

		float background_color_r = 0.2f;
		float background_color_g = 0.2f;
		float background_color_b = 0.2f;


		float background_rotation = 0.0f;
		float background_opacity = 0.0f;

		float background_gradientOpacity = 1.0f;
		float background_blurRadius = 0.0f;
		//int background_blurSteps = 0;
	};
}

#endif // !SCENE_SETTINGS_H
