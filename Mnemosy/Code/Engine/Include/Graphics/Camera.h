#ifndef CAMERA_H
#define CAMERA_H

#include "Include/Graphics/Transform.h"
#include <glm/glm.hpp>

namespace mnemosy::graphics
{

	class Camera
	{
	public:
		Camera(const uint16_t renderScreenWidth, const uint16_t renderScreenHeight);
		~Camera() = default;

		void SetScreenSize(const uint16_t width,const uint16_t height);

		const glm::mat4 GetViewMatrix();
		const glm::mat4 GetProjectionMatrix();
	
		Transform transform;
	private:
		uint16_t m_screenWidth = 0;
		uint16_t m_screenHeight = 0;
	};

} // mnemosy::graphics

#endif // !CAMERA_H


