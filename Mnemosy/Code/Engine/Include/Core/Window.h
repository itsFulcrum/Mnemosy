#ifndef WINDOW_H
#define WINDOW_H

struct GLFWwindow;

namespace mnemosy::core
{
	struct ViewportData
	{
		// size of the image renderer to in imgui viewport window
		unsigned int width = 1;
		unsigned int height = 1;

		// position of the viewport image area relative to main glfw window
		int posX = 1;
		int posY = 1;
	};


	class Window
	{
	public:
		Window(const char* WindowTitle);
		~Window();

		void Shutdown();


		GLFWwindow& GetWindow() { return *m_pWindow; }
		//GLFWwindow* GetRawWindowPointer() { return m_pWindow; }
		unsigned int GetWindowWidth() { return m_currentWindowWidth; }
		unsigned int GetWindowHeight() { return m_currentWindowHeight; }
		float GetAspectRatio() { return float(m_currentWindowWidth) / float(m_currentWindowHeight); }


		
		void SetViewportData(unsigned int viewWidth, unsigned int viewHeight, int viewStartPosX, int viewStartPosY);
		ViewportData GetViewportData()		{ return m_viewportData;}
		unsigned int GetViewportWidth()		{ return m_viewportData.width; }
		unsigned int GetViewportHeight()	{ return m_viewportData.height; }
		unsigned int GetViewportPosX()		{ return m_viewportData.posX; }
		unsigned int GetViewportPosY()		{ return m_viewportData.posY; }
		

		void SetWindowSize_InternalByIntputSystem(const unsigned int width,const unsigned int height);

	private:
		unsigned int m_currentWindowWidth = 0;
		unsigned int m_currentWindowHeight = 0;


		ViewportData m_viewportData;

		GLFWwindow* m_pWindow = nullptr;


	};

}
#endif // !WINDOW_H
