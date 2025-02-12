#include "Include/Core/Window.h"

#include "Include/MnemosyConfig.h"
#include "Include/Core/Log.h"

#include "Include/Graphics/Utils/Picture.h"
//#include "Include/Graphics/Image.h"

#include <string>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace mnemosy::core
{
	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
		GLsizei length, const char* message, const void* userParam)
	{
		//return;
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::string SourceString = "";
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:				SourceString = "API "; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		SourceString = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	SourceString = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:		SourceString = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:		SourceString = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:				SourceString = "Other"; break;
		}

		std::string TypeString = "";
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               TypeString = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: TypeString = "Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  TypeString = "Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         TypeString = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         TypeString = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              TypeString = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          TypeString = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           TypeString = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               TypeString = "Other"; break;
		}

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         MNEMOSY_CRITICAL	("OpenGL Debug: ErrorCode: ({}) Message: {}, Source: {}, Type: {}", id, message, SourceString, TypeString); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       MNEMOSY_ERROR		("OpenGL Debug: ErrorCode: ({}) Message: {}, Source: {}, Type: {}", id, message, SourceString, TypeString); break;
		case GL_DEBUG_SEVERITY_LOW:          MNEMOSY_WARN		("OpenGL Debug: ErrorCode: ({}) Message: {}, Source: {}, Type: {}", id, message, SourceString, TypeString); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: MNEMOSY_INFO		("OpenGL Debug: ErrorCode: ({}) Message: {}, Source: {}, Type: {}", id, message, SourceString, TypeString); break;
		}

		MNEMOSY_BREAK;
	}



	void Window::Init(const char* WindowTitle){

		m_currentWindowWidth = MNEMOSY_SRC_WINDOW_WIDTH;
		m_currentWindowHeight = MNEMOSY_SRC_WIDNOW_HEIGHT;

		m_pWindow = nullptr;

		m_viewportData.height = 1;
		m_viewportData.width = 1;
		m_viewportData.posX = 1;
		m_viewportData.posY = 1;

		// setup window
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
		glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef MNEMOSY_CONFIG_DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif // MNEMOSY_CONFIG_DEBUG


		// create GLFW window
		m_pWindow = glfwCreateWindow(m_currentWindowWidth, m_currentWindowHeight, WindowTitle, nullptr, nullptr);
		
		if (m_pWindow == nullptr)
		{
			MNEMOSY_ERROR("Failed to create GLFW window");
			Shutdown();
			return;
		}
		glfwMakeContextCurrent(m_pWindow);
		
		// enable vsync by default
		glfwSwapInterval(1);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			MNEMOSY_ERROR("Failed to initialize Glad");
			Shutdown();
		}


		// using STB_Read() because glfw image apparently expects 8 bit data and ReadPng outputs the highest possible bit depth while STB_Read converts to bit
		graphics::PictureError err;
		graphics::PictureInfo picInfo = graphics::Picture::STB_Read(err, "../Resources/Textures/mnemosy_icon.png",false);

		if (err.wasSuccessfull) {

			GLFWimage glfwImages[1];
			glfwImages[0].width		= picInfo.width;
			glfwImages[0].height	= picInfo.height;
			glfwImages[0].pixels	= (unsigned char*)picInfo.pixels;
			glfwSetWindowIcon(m_pWindow,1,glfwImages);

			if(picInfo.pixels)
				free(picInfo.pixels);
		}
		else {
			MNEMOSY_WARN("Unable To Set Window Icon: {}", err.what);
		}


		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


		// Setup openGl settings

		//glEnable(GL_DOUBLEBUFFER); // idk if this does anything
		glEnable(GL_MULTISAMPLE);
		//enable depth testing
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//MNEMOSY_DEBUG("glBindFramebuffer 0");
		glDisable(GL_FRAMEBUFFER_SRGB); // srgb is manually handled in the shaders

		// draw Polygons opaque (Fully shaded)
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//MNEMOSY_DEBUG("glViewport width {}, height {} ", m_currentWindowWidth,m_currentWindowHeight);
		glViewport(0, 0, m_currentWindowWidth, m_currentWindowHeight);

		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
#ifdef MNEMOSY_CONFIG_DEBUG
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
#endif // MNEMOSY_CONFIG_DEBUG
	}

	void Window::Shutdown()
	{
		glfwTerminate();

	}
	void Window::SetViewportData(unsigned int viewWidth, unsigned int viewHeight, int viewStartPosX, int viewStartPosY)
	{
		m_viewportData.width = viewWidth;
		m_viewportData.height = viewHeight;
		
		if (viewWidth <= 0)
		{
			m_viewportData.width = 2;
		}
		if (viewHeight <= 0)
		{
			m_viewportData.height = 2;
		}
		
		m_viewportData.posX = viewStartPosX;
		m_viewportData.posY = viewStartPosY;
	}
	void Window::SetWindowSize_InternalByIntputSystem(const unsigned int width, const unsigned int height) {
		m_currentWindowWidth = width;
		m_currentWindowHeight = height;
	}

	void Window::EnableVsync(bool enable)
	{
		if (enable)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}

	}

} // mnemosy::core