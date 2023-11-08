#ifndef GRAPHICAL_USER_INTERFACE_H
#define GRAPHICAL_USER_INTERFACE_H

#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <stdio.h>

#include <memory>



#include <view/gui/GuiPanel.h>
#include <view/gui/GuiPanelViewport.h>
#include <view/gui/GuiPanelSettings.h>


class GraphicalUserInterface
{
public:
	GraphicalUserInterface(GLFWwindow* window);
	~GraphicalUserInterface();

	void Init(GLFWwindow* window);
	void UpdateWindowPosition(GLFWwindow* window);
	void Render();
	void EndFrame();
	bool IsMouseOverViewport();


	unsigned int viewportRenderTextureID;
	unsigned int viewportWidth = 0;
	unsigned int viewportHeight = 0;


private:
	int* m_MainWindowPositionX = new int;
	int* m_MainWindowPositionY = new int;

	ImVec2 m_MousePosition = ImVec2(0, 0); 
	
	ImVec2 m_WindowStartPosition = ImVec2(0, 0);
	ImVec2 m_ViewportStartPosition = ImVec2(0, 0);
	
	bool isViewportHovered = false;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool show_demo_window = true;
	bool show_another_window = false;
	bool show_Viewport = true;
	bool show_Settings = true;


	GuiPanel* p; 

	


};




#endif