#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

struct GLFWwindow;

namespace mnemosy::core
{
	struct ViewportData
	{
		// size of the image renderer to in imgui viewport window
		uint16_t width = 1;
		uint16_t height = 1;

		// position of the viewport image area relative to main glfw window
		int posX = 1;
		int posY = 1;
	};


	class Window
	{
	public:
		Window()  = default;
		~Window() = default;


		void Init(const char* WindowTitle);
		void Shutdown();


		GLFWwindow& GetWindow() { return *m_pWindow; }
		//GLFWwindow* GetRawWindowPointer() { return m_pWindow; }
		const uint16_t GetWindowWidth()  const { return m_currentWindowWidth; }
		const uint16_t GetWindowHeight() const { return m_currentWindowHeight; }
		const float GetAspectRatio() const { return float(m_currentWindowWidth) / float(m_currentWindowHeight); }

		const ViewportData& GetViewportData()	const { return m_viewportData;}
		const unsigned int GetViewportWidth()	const { return m_viewportData.width; }
		const unsigned int GetViewportHeight()	const { return m_viewportData.height; }
		const int GetViewportPosX()	const { return m_viewportData.posX; }
		const int GetViewportPosY()	const { return m_viewportData.posY; }
		
		void SetViewportData(const unsigned int viewWidth,const unsigned int viewHeight,const int viewStartPosX,const int viewStartPosY);
		void SetWindowSize_InternalByIntputSystem(const unsigned int width,const unsigned int height);
		void EnableVsync(const bool enable);

	private:
		uint16_t m_currentWindowWidth = 0;
		uint16_t m_currentWindowHeight = 0;

		ViewportData m_viewportData;

		GLFWwindow* m_pWindow = nullptr;
	};

} // mnemosy::core

#endif // !WINDOW_H
