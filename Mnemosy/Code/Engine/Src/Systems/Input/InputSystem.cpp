#include "Include/Systems/Input/InputSystem.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Core/Log.h"

namespace mnemosy::systems
{

	// glfw callbacks
	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		//MNEMOSY_WARN("InputSystem::framebuffer_size_callback: Width: {} Height: {} ", width, height);

		InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
		inputSystem->UpdateWindowResizeInputs(width, height);

		inputSystem = nullptr;
		delete inputSystem;
	}
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
		inputSystem->UpdateMouseButtonInputs(button, action, mods);
		inputSystem = nullptr;
		delete inputSystem;
	}
	void mouse_cursor_callback(GLFWwindow* window, double posX, double posY)
	{
		InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
		inputSystem->UpdateMouseCursorInputs(posX, posY);
		inputSystem = nullptr;
		delete inputSystem;
	}
	void mouse_scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
	{
		InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
		inputSystem->UpdateMouseScrollInputs(offsetX, offsetY);
		inputSystem = nullptr;
		delete inputSystem;
	}

	void drop_callback(GLFWwindow* window, int count, const char** paths)
	{

		InputSystem* inputSystem = (InputSystem*)glfwGetWindowUserPointer(window);
		//inputSystem->UpdateMouseScrollInputs(offsetX, offsetY);
		inputSystem->UpdateDropCallback(count,paths);
		inputSystem = nullptr;
		delete inputSystem;
	}

// inputSystem.RegisterKeyboardInput(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, true, std::bind(&SceneInputController::OnKeyPressed_LEFT_SHIFT, this, std::placeholders::_1));
// #define MNEMOSY_ASSERT(x, msg)	if((x)) {} else { MNEMOSY_CRITICAL("ASSERT - {}\n\t{}\n\tin file {}\n\ton line {}", #x, msg, __FILE__, __LINE__); MNEMOSY_BREAK}


	InputSystem::InputSystem()
	{
		m_pWindow = &MnemosyEngine::GetInstance().GetWindow().GetWindow();
		glfwSetWindowUserPointer(m_pWindow, this);

		glfwSetFramebufferSizeCallback(m_pWindow, framebuffer_size_callback);
		glfwSetMouseButtonCallback(m_pWindow, mouse_button_callback);
		glfwSetScrollCallback(m_pWindow, mouse_scroll_callback);
		glfwSetCursorPosCallback(m_pWindow, mouse_cursor_callback);

		glfwSetDropCallback(m_pWindow, drop_callback);

	}

	InputSystem::~InputSystem()
	{

		m_keyboardEntries.clear();
		m_mouseButtonEntries.clear();
		m_mouseCursorEntries.clear();
		m_mouseScrollEntries.clear();
		m_windowResizeEntries.clear();
		m_dropEntries.clear();

		m_pWindow = nullptr;
	}

	int InputSystem::RegisterKeyboardInput(int glfwKeyboardKey, int glfwPressEvent, bool callbackOnlyOnce, TCallbackSignatureDouble callbackFunction)
	{

		m_keyboardIdCounter++;

		KeyboardInputEntry newEntry;
		newEntry.id = m_keyboardIdCounter++;
		newEntry.glfw_keyToListenTo = glfwKeyboardKey;
		newEntry.glfw_pressEventToListenTo = glfwPressEvent;
		newEntry.callbackOnlyOnce = callbackOnlyOnce;
		newEntry.callbackFunction = callbackFunction;

		m_keyboardEntries.push_back(newEntry);

		return newEntry.id;
	}

	int InputSystem::RegisterMouseButtonInput(int glfwMouseButton, int glfwPressEvent, TCallbackSignatureDouble callbackFunction)
	{
		m_mouseButtonIdCounter++;

		MouseButtonInputEntry newEntry;
		newEntry.id = m_mouseButtonIdCounter;
		newEntry.glfw_mouseButtonToListenTo = glfwMouseButton;
		newEntry.glfw_pressEventToListenTo = glfwPressEvent;
		newEntry.callbackFunction = callbackFunction;

		m_mouseButtonEntries.push_back(newEntry);

		return newEntry.id;
	}

	int InputSystem::RegisterMouseCursorInput(TCallbackSignatureDouble5 callbackFunction)
	{
		m_mouseCursorIdCounter++;

		MouseCursorInputEntry newEntry;
		newEntry.id = m_mouseCursorIdCounter;
		newEntry.callbackFunction = callbackFunction;

		m_mouseCursorEntries.push_back(newEntry);

		return newEntry.id;
	}

	int InputSystem::RegisterMouseScrollInput(TCallbackSignatureDouble3 callbackFunction)
	{
		m_mouseScrollIdCounter++;

		MouseScrollInputEntry newEntry;
		newEntry.id = m_mouseScrollIdCounter;
		newEntry.callbackFunction = callbackFunction;

		m_mouseScrollEntries.push_back(newEntry);

		return newEntry.id;
	}

	int InputSystem::RegisterWindowResizeInput(TCallbackSignatureInt2 callbackFunction)
	{
		m_windowResizeIdCounter++;

		WindowResizeInputEntry newEntry;
		newEntry.id = m_windowResizeIdCounter;
		newEntry.callbackFunction = callbackFunction;

		m_windowResizeEntries.push_back(newEntry);

		return newEntry.id;
	}

	int InputSystem::RegisterDropInput(TCallbackSingatureDrop callbackFunction)
	{
		m_dropIdCounter++;

		DropInputEntry newEntry;
		newEntry.id = m_dropIdCounter;
		newEntry.callbackFunction = callbackFunction;

		m_dropEntries.push_back(newEntry);

		return 0;
	}


	void InputSystem::UnregisterKeyboardInput(int callbackId)
	{
		for (int i = 0; i < m_keyboardEntries.size(); i++)
		{
			if (m_keyboardEntries[i].id == callbackId)
			{
				m_keyboardEntries.erase(m_keyboardEntries.begin() + i);
				return;
			}
		}
		// could maybe do m_keyboardIdCounter--; ??
		return;
	}

	void InputSystem::UnregisterMouseButtonInput(int callbackId)
	{
		for (int i = 0; i < m_mouseButtonEntries.size(); i++)
		{
			if (m_mouseButtonEntries[i].id == callbackId)
			{
				m_mouseButtonEntries.erase(m_mouseButtonEntries.begin() + i);
				return;
			}
		}
		return;
	}

	void InputSystem::UnregisterMouseCursorInput(int callbackId)
	{
		for (int i = 0; i < m_mouseCursorEntries.size(); i++)
		{
			if (m_mouseCursorEntries[i].id == callbackId)
			{
				m_mouseCursorEntries.erase(m_mouseCursorEntries.begin() + i);
				return;
			}
		}
		return;
	}

	void InputSystem::UnregisterMouseScrollInput(int callbackId)
	{
		for (int i = 0; i < m_mouseScrollEntries.size(); i++)
		{
			if (m_mouseScrollEntries[i].id == callbackId)
			{
				m_mouseScrollEntries.erase(m_mouseScrollEntries.begin() + i);
				return;
			}
		}
		return;
	}

	void InputSystem::UnregisterWindowResizeInput(int callbackId)
	{
		for (int i = 0; i < m_windowResizeEntries.size(); i++)
		{
			if (m_windowResizeEntries[i].id == callbackId)
			{
				m_windowResizeEntries.erase(m_windowResizeEntries.begin() + i);
				return;
			}
		}
		return;
	}

	void InputSystem::UnregisterDropInput(int callbackId)
	{
		for (int i = 0; i < m_dropEntries.size(); i++)
		{
			if (m_dropEntries[i].id == callbackId)
			{
				m_dropEntries.erase(m_dropEntries.begin() + i);
				return;
			}
		}
		return;

	}

	// Update on calback
	void InputSystem::UpdateKeyboardInputs()
	{
		if (!m_processUserInputs)
			return;


		for (KeyboardInputEntry& entry : m_keyboardEntries)
		{

			if (entry.callbackOnlyOnce)
			{
				bool isPressedNow = glfwGetKey(m_pWindow, entry.glfw_keyToListenTo) == GLFW_PRESS;

				if (entry.wasPressedLastFrame && !isPressedNow)
				{
					//release key
					if (entry.glfw_pressEventToListenTo == GLFW_RELEASE)
					{
						entry.callbackFunction(m_deltaSeconds);
					}
				}
				else if (!entry.wasPressedLastFrame && isPressedNow)
				{
					//pressed key
					if (entry.glfw_pressEventToListenTo == GLFW_PRESS)
					{
						entry.callbackFunction(m_deltaSeconds);
					}
				}

				entry.wasPressedLastFrame = isPressedNow;
			}
			else // !entry.callbackOnlyOnce
			{
				if (glfwGetKey(m_pWindow, entry.glfw_keyToListenTo) == entry.glfw_pressEventToListenTo)
				{
					entry.callbackFunction(m_deltaSeconds);
				}
			}



		}
	}

	void InputSystem::UpdateMouseButtonInputs(int button, int action, int mods)
	{
		if (!m_processUserInputs)
			return;


		for (MouseButtonInputEntry& entry : m_mouseButtonEntries)
		{
			if (button == entry.glfw_mouseButtonToListenTo && action == entry.glfw_pressEventToListenTo)
			{
				entry.callbackFunction(m_deltaSeconds);
			}
		}
	}

	void InputSystem::UpdateMouseCursorInputs(double xPos, double yPos)
	{
		if (!m_processUserInputs)
			return;


		if (m_firstMouseInput)
		{
			m_mouseLastFrameXPos = xPos;
			m_mouseLastFrameYPos = yPos;
			m_firstMouseInput = false;
		}

		// called each time the mouse is moved
		double mouseOffsetX = xPos - m_mouseLastFrameXPos;
		double mouseOffsetY = m_mouseLastFrameYPos - yPos;


		m_mouseLastFrameXPos = xPos;
		m_mouseLastFrameYPos = yPos;


		for (MouseCursorInputEntry& entry : m_mouseCursorEntries)
		{
			entry.callbackFunction(m_deltaSeconds, xPos, yPos, mouseOffsetX, mouseOffsetY);
		}

	}

	void InputSystem::UpdateMouseScrollInputs(double offsetX, double offsetY)
	{
		if (!m_processUserInputs)
			return;

		for (MouseScrollInputEntry& entry : m_mouseScrollEntries)
		{
			entry.callbackFunction(m_deltaSeconds, offsetX, offsetY);
		}
	}

	void InputSystem::UpdateWindowResizeInputs(int width, int height)
	{
		//if (!m_processUserInputs)
		//	return;
		
		// hardcoded to update the window class maybe not ideal could register a callback from Engine class 
		// but this ensures that it'll get updated first
		MnemosyEngine::GetInstance().GetWindow().SetWindowSize_InternalByIntputSystem(width, height);

		for (WindowResizeInputEntry& entry : m_windowResizeEntries)
		{
			entry.callbackFunction(width, height);
		}
	}

	void InputSystem::UpdateDropCallback(int count, const char** paths)
	{
		//MNEMOSY_DEBUG("InputSystem::UpdateDropCallback: Drop Count {}",count);
		
		if (!m_dropEntries.empty()) {

			int dropCount = count;
			std::vector<std::string> dropedPathsStrings;
			// populate vector
			for (int i = 0; i < count; i++) {
				dropedPathsStrings.push_back(paths[i]);
			}


			for (int i = 0; i < m_dropEntries.size();i++) {

				m_dropEntries[i].callbackFunction(dropCount, dropedPathsStrings);
			}

			// clear vector
			dropedPathsStrings.clear();
		}

	}

	void InputSystem::Update(double deltaSeconds)
	{
		m_deltaSeconds = deltaSeconds;

		if (!m_processUserInputs)
			return;
		
		UpdateKeyboardInputs();
	}

	void InputSystem::LockCursor()
	{
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void InputSystem::UnlockCursor()
	{
		
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	void InputSystem::HideCursor()
	{
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}

	bool InputSystem::IsButtonPressed(int glfwKeyboardKey) {
		if (!m_processUserInputs)
			return false;

		return glfwGetKey(m_pWindow, glfwKeyboardKey) == GLFW_PRESS;
	}

	bool InputSystem::IsMouseButtonPressed(int glfwMouseBtnKey) {
		if (!m_processUserInputs)
			return false;
		
		return glfwGetMouseButton(m_pWindow, glfwMouseBtnKey) == GLFW_PRESS;

	}

	void InputSystem::HandleMouseOverViewport() {
		// mouse cursor position relative to the main glfw window
		double cursorWindowPosX_double = 0;
		double cursorWindowPosY_double = 0;
		glfwGetCursorPos(m_pWindow, &cursorWindowPosX_double, &cursorWindowPosY_double); 

		// position of the main glfw window on the desktop
		int windowStartPosX = 0; int windowStartPosY = 0;
		glfwGetWindowPos(m_pWindow, &windowStartPosX, &windowStartPosY);
		// window size of the main glfw Window
		int windowSizeX = 0; int windowSizeY = 0;
		glfwGetWindowSize(m_pWindow, &windowSizeX, &windowSizeY);


		// absolute mouse cursor position on the desktop
		int mouseScreenPosX = windowStartPosX + (int)cursorWindowPosX_double;
		int mouseScreenPosY = windowStartPosY + (int)cursorWindowPosY_double;

		// get viewport data from MnemosyWindow
		core::ViewportData viewportData = MnemosyEngine::GetInstance().GetWindow().GetViewportData();

		// absolute position of viewport on the monitor
		int viewportStartPosX = windowStartPosX + viewportData.posX;		
		int viewportStartPosY = windowStartPosY + viewportData.posY;

		int viewportWidth = viewportData.width;
		int viewportHeight = viewportData.height;
		
		// set the mouse postion to opposing site of the viewport if it get out of bounds
		int padding = 10;
		// LEFT
		if (mouseScreenPosX <= (viewportStartPosX - padding))
		{
			double newPosX = (viewportStartPosX + viewportWidth - padding) - windowStartPosX;
			glfwSetCursorPos(m_pWindow, newPosX, (double)mouseScreenPosY - windowStartPosY);
			m_mouseLastFrameXPos = newPosX;
			return;
		}
		// RIGHT
		if (mouseScreenPosX >= (viewportStartPosX + viewportWidth - padding))
		{
			double newPosX = viewportStartPosX - padding - windowStartPosX;
			glfwSetCursorPos(m_pWindow, newPosX, (double)mouseScreenPosY - windowStartPosY);
			m_mouseLastFrameXPos = newPosX;
			return;
		}

		// TOP
		if (mouseScreenPosY <= (viewportStartPosY - padding))
		{
			double newPosY = viewportStartPosY + viewportHeight - padding - windowStartPosY;
			glfwSetCursorPos(m_pWindow, (double)mouseScreenPosX - windowStartPosX, newPosY);
			m_mouseLastFrameYPos = newPosY;
			return;
		}
		// BOTTOM
		if (mouseScreenPosY >= (viewportStartPosY + viewportHeight - padding))
		{
			double newPosY = viewportStartPosY - padding - windowStartPosY;
			glfwSetCursorPos(m_pWindow, (double)mouseScreenPosX - windowStartPosX, newPosY);
			m_mouseLastFrameYPos = newPosY;
			return;
		}
	}

} // !mnemosy::gui