#ifndef CAMERA_H
#define CAMERA_H

#include "Include/Graphics/Transform.h"
#include <glm/glm.hpp>

namespace mnemosy::graphics
{
	struct CameraSettings
	{
		float fov = 45.0f;
		float nearClip = 0.1f;
		float farClip = 500.0f;
	};

	class Camera
	{
	public:
		Camera(unsigned int renderScreenWidth, unsigned int renderScreenHeight);
		~Camera();

		void SetScreenSize(unsigned int width, unsigned int height);

		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjectionMatrix();
	
		CameraSettings settings;
		Transform transform;
	private:
		unsigned int m_screenWidth = 0;
		unsigned int m_screenHeight = 0;
	};

} // mnemosy::graphics

#endif // !CAMERA_H


