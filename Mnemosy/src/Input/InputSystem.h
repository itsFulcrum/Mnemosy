#pragma once
#include <GLFW/glfw3.h>


#include <vector>
#include <functional>

using TCallbackSignature = std::function<void()>;
using TCallbackSignatureDouble = std::function<void(double deltaSeconds)>;
using TCallbackSignatureDouble3 = std::function<void(double deltaSeconds, double x, double y)>;
using TCallbackSignatureDouble5 = std::function<void(double deltaSeconds, double x, double y,double z, double w)>;
using TCallbackSignatureInt2 = std::function<void( int x, int y)>;


struct KeyboardInputEntry
{
	int id = 0;
	bool wasPressedLastFrame = false;
	bool callbackOnlyOnce = true;
	int glfw_pressEventToListenTo = GLFW_PRESS;
	int glfw_keyToListenTo = GLFW_KEY_SPACE;
	TCallbackSignatureDouble callbackFunction;
};
struct MouseButtonInputEntry
{
	int id = 0;
	int glfw_pressEventToListenTo = GLFW_PRESS;
	int glfw_mouseButtonToListenTo = GLFW_MOUSE_BUTTON_LEFT;
	TCallbackSignatureDouble callbackFunction;
};
struct MouseCursorInputEntry
{
	int id = 0;
	TCallbackSignatureDouble5 callbackFunction;
};
struct MouseScrollInputEntry
{
	int id = 0;
	TCallbackSignatureDouble3 callbackFunction;
};
struct WindowResizeInputEntry
{
	int id = 0;
	TCallbackSignatureInt2 callbackFunction;
};


class InputSystem
{
public:
	InputSystem(GLFWwindow* window);
	~InputSystem();

	int RegisterKeyboardInput(int glfwKeyboardKey, int glfwPressEvent, bool callbacksOnlyOnce, TCallbackSignatureDouble callbackFunction);
	int RegisterMouseButtonInput(int glfwMouseButton, int glfwPressEvent, TCallbackSignatureDouble callbackFunction);
	int RegisterMouseCursorInput(TCallbackSignatureDouble5 callbackFunction);
	int RegisterMouseScrollInput(TCallbackSignatureDouble3 callbackFunction);
	int RegisterWindowResizeInput(TCallbackSignatureInt2 callbackFunction);


	void UnregisterKeyboardInput(int callbackId);
	void UnregisterMouseButtonInput(int callbackId);
	void UnregisterMouseCursorInput(int callbackId);
	void UnregisterMouseScrollInput(int callbackId);
	void UnregisterWindowResizeInput(int callbackId);
	

	void UpdateKeyboardInputs();
	void UpdateMouseButtonInputs(int button, int action, int mods);
	void UpdateMouseCursorInputs(double posX, double posY);
	void UpdateMouseScrollInputs(double offsetX, double offsetY);
	void UpdateWindowResizeInputs(int width, int height);

	void Update(double deltaSeconds);

	// currently not in useKeep for latermaybe
	void HandleMouseOverViewport(int viewportStartScreenPosX, int viewportStartScreenPosY, int _viewportWidth, int _viewportHeight);
private:

	void SetWindowSizeInternal(int width,int height);
private:
	GLFWwindow* m_window;
	double m_deltaSeconds = 0;

	bool m_processUserInputs = true;

	int m_keyboardIdCounter = 0;
	int m_mouseButtonIdCounter = 0;
	int m_mouseCursorIdCounter = 0;
	int m_mouseScrollIdCounter = 0;
	int m_windowResizeIdCounter = 0;

	std::vector<KeyboardInputEntry> m_keyboardEntries;
	std::vector<MouseButtonInputEntry> m_mouseButtonEntries;
	std::vector<MouseCursorInputEntry> m_mouseCursorEntries;
	std::vector<MouseScrollInputEntry> m_mouseScrollEntries;
	std::vector<WindowResizeInputEntry> m_windowResizeEntries;


	double m_mouseLastFrameXPos = 0.0;
	double m_mouseLastFrameYPos = 0.0;

	bool firstMouseInput = true;

	int m_windowWidth = 0;
	int m_windowHeight = 0;

};