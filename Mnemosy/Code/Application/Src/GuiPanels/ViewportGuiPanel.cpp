#include "Include/GuiPanels/ViewportGuiPanel.h"

#include "Include/Application.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/Window.h"
#include "External/ImGui/imgui.h"

namespace mnemosy::gui
{
	ViewportGuiPanel::ViewportGuiPanel()
	{
		panelName = "Viewport";
	}

	void ViewportGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		ImGui::Begin(panelName.c_str(), &showPanel);

		// width and height of the entrire imGui window including top bar and left padding
		ImVec2 windowSize = ImGui::GetWindowSize();
		// start position relative to the main glfw window
		ImVec2 windowPos = ImGui::GetWindowPos();
		// available size inside the window  use this to define the width and height of the texture image
		// if i subtract this from GetWindowSize() i should get the start coordinates from the image relative to the panel window
		ImVec2 avail_size = ImGui::GetContentRegionAvail();
		ImVec2 imageSize = avail_size;

		// start position of viewport relative to the main glfw window
		int viewportPosX =int( windowPos.x + (windowSize.x - avail_size.x));
		int viewportPosY =int( windowPos.y + (windowSize.y - avail_size.y));


		unsigned int vSizeX = (unsigned int)imageSize.x;
		if(imageSize.x <= 0)
			vSizeX = 1;
		unsigned int vSizeY = (unsigned int)imageSize.y;
		if(imageSize.y <= 0)
			vSizeY = 1;


		ENGINE_INSTANCE().GetWindow().SetViewportData(vSizeX, vSizeY, viewportPosX, viewportPosY);

		//ImGui::Text("Width %i, Height %i", viewportWidth, viewportHeight);
		//ImGui::Text("availX %f, availY %f", avail_size.x, avail_size.y);
		//ImGui::Text("availX %f, availY %f", avail_size.x, avail_size.y);
		//ImGui::Text("viewportRenderTextureID = %d", viewportRenderTextureID);

		ImGui::Image((void*)ENGINE_INSTANCE().GetRenderer().GetRenderTextureId(), imageSize , ImVec2(0, 1), ImVec2(1, 0));		
		if (ImGui::IsWindowHovered())
		{
			ImGui::CaptureMouseFromApp(false);
			ImGui::CaptureKeyboardFromApp(false);
		}


		ImGui::End();
	}
}