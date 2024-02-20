#ifndef GRAPHICS_CAMERA_H
#define GRPAHICS_CAMERA_H

#include "Engine/Include/Graphics/Transform.h"

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
	

		CameraSettings settings;

		Camera(unsigned int renderScreenWidth, unsigned int renderScreenHeight);
		~Camera();

		void SetScreenSize(unsigned int width, unsigned int height);

		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjectionMatrix();
	
		Transform transform;
	private:
		unsigned int m_screenWidth = 0;
		unsigned int m_screenHeight = 0;
	};



}

#endif // !GRAPHICS_CAMERA_H


