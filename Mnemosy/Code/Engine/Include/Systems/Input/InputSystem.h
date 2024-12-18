#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <stdint.h>
#include <functional>

namespace mnemosy::systems
{
#define REGISTER_KEYBOARD_INPUT(keyboardKey,pressEvent,callOnlyOnce,functionPointer)	RegisterKeyboardInput(keyboardKey,pressEvent,callOnlyOnce, std::bind(functionPointer, this, std::placeholders::_1))
#define REGISTER_MOUSE_BUTTON_INPUT(mouseButton,pressEvent,functionPointer)				RegisterMouseButtonInput(mouseButton,pressEvent,std::bind(functionPointer, this, std::placeholders::_1))
#define REGISTER_MOUSE_CURSOR_INPUT(functionPointer)									RegisterMouseCursorInput(std::bind(functionPointer, this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5))
#define REGISTER_MOUSE_SCROLL_INPUT(functionPointer)									RegisterMouseScrollInput(std::bind(functionPointer, this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
#define REGISTER_WINDOW_RESIZE_INPUT(functionPointer)									RegisterWindowResizeInput(std::bind(functionPointer, this,std::placeholders::_1, std::placeholders::_2))
#define REGISTER_DROP_INPUT(functionPointer)											RegisterDropInput(std::bind(functionPointer, this,std::placeholders::_1, std::placeholders::_2))

	//inputSystem.RegisterMouseButtonInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, std::bind(&SceneInputController::OnMouseButtonPressed_RIGHT, this, std::placeholders::_1));
	// glfw callbacks
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void mouse_cursor_callback(GLFWwindow* window, double posX, double posY);
	void mouse_scroll_callback(GLFWwindow* window, double offsetX, double offsetY);
	void drop_callback(GLFWwindow* window, int count, const char** paths);


	using TCallbackSignature = std::function<void()>;
	using TCallbackSignatureDouble = std::function<void(double deltaSeconds)>;
	using TCallbackSignatureDouble3 = std::function<void(double deltaSeconds, double x, double y)>;
	using TCallbackSignatureDouble5 = std::function<void(double deltaSeconds, double x, double y, double z, double w)>;
	using TCallbackSignatureInt2 = std::function<void(int x, int y)>;
	using TCallbackSingatureDrop = std::function<void(int count, std::vector<std::string>& paths)>;

	struct KeyboardInputEntry
	{
		TCallbackSignatureDouble callbackFunction;
		unsigned int glfw_pressEventToListenTo = GLFW_PRESS;
		unsigned int glfw_keyToListenTo = GLFW_KEY_SPACE;
		uint8_t id = 0;
		bool wasPressedLastFrame = false;
		bool callbackOnlyOnce = true;
	};
	struct MouseButtonInputEntry
	{
		uint8_t id = 0;
		unsigned int glfw_pressEventToListenTo = GLFW_PRESS;
		unsigned int glfw_mouseButtonToListenTo = GLFW_MOUSE_BUTTON_LEFT;
		TCallbackSignatureDouble callbackFunction;
	};
	struct MouseCursorInputEntry
	{
		uint8_t id = 0;
		TCallbackSignatureDouble5 callbackFunction;
	};
	struct MouseScrollInputEntry
	{
		uint8_t id = 0;
		TCallbackSignatureDouble3 callbackFunction;
	};
	struct WindowResizeInputEntry
	{
		uint8_t id = 0;
		TCallbackSignatureInt2 callbackFunction;
	};
	struct DropInputEntry {
		uint8_t id = 0;
		TCallbackSingatureDrop callbackFunction;
	};

	class InputSystem
	{
	public:
		InputSystem() = default;
		~InputSystem() = default;

		void Init();
		void Shutdown();


		int RegisterKeyboardInput(int glfwKeyboardKey, int glfwPressEvent, bool callbacksOnlyOnce, TCallbackSignatureDouble callbackFunction);
		int RegisterMouseButtonInput(int glfwMouseButton, int glfwPressEvent, TCallbackSignatureDouble callbackFunction);
		int RegisterMouseCursorInput(TCallbackSignatureDouble5 callbackFunction);
		int RegisterMouseScrollInput(TCallbackSignatureDouble3 callbackFunction);
		int RegisterWindowResizeInput(TCallbackSignatureInt2 callbackFunction);
		int RegisterDropInput(TCallbackSingatureDrop callbackFunction);

		void UnregisterKeyboardInput(int callbackId);
		void UnregisterMouseButtonInput(int callbackId);
		void UnregisterMouseCursorInput(int callbackId);
		void UnregisterMouseScrollInput(int callbackId);
		void UnregisterWindowResizeInput(int callbackId);
		void UnregisterDropInput(int callbackId);

		void UpdateKeyboardInputs();
		void UpdateMouseButtonInputs(int button, int action, int mods);
		void UpdateMouseCursorInputs(double posX, double posY);
		void UpdateMouseScrollInputs(double offsetX, double offsetY);
		void UpdateWindowResizeInputs(int width, int height);
		void UpdateDropCallback(int count, const char** paths);

		void Update(double deltaSeconds);


		bool IsButtonPressed(int glfwKeyboardKey);
		bool IsMouseButtonPressed(int glfwMouseBtnKey);

		void LockCursor();
		void UnlockCursor();
		void HideCursor();

		void ProcessUserInputs() { m_processUserInputs = true; }
		void DontProcessUserInputs() { m_processUserInputs = false; }

		void HandleMouseOverViewport();
	
	private:
		GLFWwindow* m_pWindow;
		
		double m_deltaSeconds = 0;
		double m_mouseLastFrameXPos = 0.0;
		double m_mouseLastFrameYPos = 0.0;

		bool m_processUserInputs = true;
		bool m_firstMouseInput = true;

		// this should be higher probably but for mnemosy purposes 255 different registers is more then enough
		uint8_t m_keyboardIdCounter = 0;
		uint8_t m_mouseButtonIdCounter = 0;
		uint8_t m_mouseCursorIdCounter = 0;
		uint8_t m_mouseScrollIdCounter = 0;
		uint8_t m_windowResizeIdCounter = 0;
		uint8_t m_dropIdCounter = 0;

		std::vector<KeyboardInputEntry> m_keyboardEntries;
		std::vector<MouseButtonInputEntry> m_mouseButtonEntries;
		std::vector<MouseCursorInputEntry> m_mouseCursorEntries;
		std::vector<MouseScrollInputEntry> m_mouseScrollEntries;
		std::vector<WindowResizeInputEntry> m_windowResizeEntries;
		std::vector<DropInputEntry> m_dropEntries;

	};

} // mnemosy::systems

#endif // !INPUT_SYSTEM_H
