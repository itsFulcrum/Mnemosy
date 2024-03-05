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
		Camera(const unsigned int renderScreenWidth, const unsigned int renderScreenHeight);
		~Camera() = default;

		void SetScreenSize(const unsigned int width,const unsigned int height);

		const glm::mat4 GetViewMatrix();
		const glm::mat4 GetProjectionMatrix();
	
		CameraSettings settings;
		Transform transform;
	private:
		unsigned int m_screenWidth = 0;
		unsigned int m_screenHeight = 0;
	};

} // mnemosy::graphics

#endif // !CAMERA_H


