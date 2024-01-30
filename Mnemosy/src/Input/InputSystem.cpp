#include "InputSystem.h"
#include <iostream>
InputSystem::InputSystem(GLFWwindow* window)
{
	m_window = window;
}

InputSystem::~InputSystem()
{
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



void InputSystem::UpdateKeyboardInputs()
{
	if (!m_processUserInputs)
		return;


	for (KeyboardInputEntry& entry : m_keyboardEntries)
	{

		if (entry.callbackOnlyOnce) 
		{
			bool isPressedNow = glfwGetKey(m_window, entry.glfw_keyToListenTo) == GLFW_PRESS;	
			
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
			if (glfwGetKey(m_window, entry.glfw_keyToListenTo) == entry.glfw_pressEventToListenTo)
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

	
	if (firstMouseInput)
	{
		m_mouseLastFrameXPos = xPos;
		m_mouseLastFrameYPos = yPos;
		firstMouseInput = false;
	}

	// called each time the mouse is moved
	double mouseOffsetX = xPos - m_mouseLastFrameXPos;
	double mouseOffsetY = m_mouseLastFrameYPos - yPos;


	m_mouseLastFrameXPos = xPos;
	m_mouseLastFrameYPos = yPos;


	for (MouseCursorInputEntry& entry : m_mouseCursorEntries)
	{
		entry.callbackFunction(m_deltaSeconds, xPos,  yPos,mouseOffsetX,mouseOffsetY);
	}

}

void InputSystem::UpdateMouseScrollInputs(double offsetX, double offsetY)
{
	if (!m_processUserInputs)
		return;

	for (MouseScrollInputEntry& entry : m_mouseScrollEntries)
	{
		entry.callbackFunction(m_deltaSeconds,offsetX, offsetY);
	}
}

void InputSystem::UpdateWindowResizeInputs(int width, int height)
{
	//if (!m_processUserInputs)
	//	return;
	SetWindowSizeInternal(width, height);

	for (WindowResizeInputEntry& entry : m_windowResizeEntries)
	{
		entry.callbackFunction(width, height);
	}
}

void InputSystem::Update(double deltaSeconds)
{
	m_deltaSeconds = deltaSeconds;
	UpdateKeyboardInputs();
}

void InputSystem::HandleMouseOverViewport(int viewportStartScreenPosX, int viewportStartScreenPosY, int _viewportWidth, int _viewportHeight)
{

	//if (!mouseLocked)
		//return;

	// glfw Sets the mouse courser position based on numbers relative to the upper left corner (0,0) of the main window area


	double cursorWindowPosX_double = 0;
	double cursorWindowPosY_double = 0;
	glfwGetCursorPos(m_window, &cursorWindowPosX_double, &cursorWindowPosY_double); // relative to the window
	
	int windowStartPosX = 0;
	int windowStartPosY = 0;
	glfwGetWindowPos(m_window, &windowStartPosX, &windowStartPosY);

	int mouseScreenPosX = windowStartPosX + (int)cursorWindowPosX_double;
	int mouseScreenPosY = windowStartPosY + (int)cursorWindowPosY_double;



	int viewportWidth = 0;
	int viewportHeight = 0;
	// Must come from outside if in use with ImGUI
	int viewportStartPosX = windowStartPosX;
	int viewportStartPosY = windowStartPosY;

	int windowSizeX = 0;
	int windowSizeY = 0;
	glfwGetWindowSize(m_window, &windowSizeX,&windowSizeY);

	viewportWidth = windowSizeX;
	viewportHeight = windowSizeY;

	if (mouseScreenPosX < viewportStartPosX)
	{
		// set mouse pos x to windowStartPos.x + windowWidth

		double newPosX = (viewportStartPosX + viewportWidth) - windowStartPosX;

		glfwSetCursorPos(m_window, newPosX, (double)mouseScreenPosY - windowStartPosY);
		m_mouseLastFrameXPos = newPosX;
	}
	if (mouseScreenPosX > (viewportStartPosX + viewportWidth))
	{
		// set mouse pos x to viewportStartPos.x
		double newPosX = viewportStartPosX - windowStartPosX;
		glfwSetCursorPos(m_window, newPosX, (double)mouseScreenPosY - windowStartPosY);

		m_mouseLastFrameXPos = newPosX;
	}


	if (mouseScreenPosY < viewportStartPosY)
	{
		// set mouse pos y to windowStartPos.y + windowheight
		double newPosY = viewportStartPosY + viewportHeight - windowStartPosY;

		glfwSetCursorPos(m_window, (double)mouseScreenPosX - windowStartPosX, newPosY);
		m_mouseLastFrameYPos = newPosY;
	}
	if (mouseScreenPosY > (viewportStartPosY + viewportHeight))
	{
		// set mouse pos y to windowStartPos.y
		double newPosY = viewportStartPosY - windowStartPosY;
		glfwSetCursorPos(m_window, (double)mouseScreenPosX - windowStartPosX, newPosY);
		m_mouseLastFrameYPos = newPosY;
	}
}

void InputSystem::SetWindowSizeInternal(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}

