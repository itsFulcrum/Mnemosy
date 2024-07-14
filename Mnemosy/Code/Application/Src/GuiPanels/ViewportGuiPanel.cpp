#include "Include/GuiPanels/ViewportGuiPanel.h"

#include "Include/Application.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/Window.h"

namespace mnemosy::gui
{
	ViewportGuiPanel::ViewportGuiPanel()
		: m_engineInstance{ MnemosyEngine::GetInstance() }
	{
		panelName = "Viewport";
	}

	void ViewportGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		ImGui::Begin(panelName, &showPanel);

		// width and height of the entrire imGui window including top bar and left padding
		m_windowSize = ImGui::GetWindowSize();
		// start position relative to the main glfw window
		m_windowPos = ImGui::GetWindowPos();
		// available size inside the window  use this to define the width and height of the texture image
		// if i subtract this from GetWindowSize() i should get the start coordinates from the image relative to the panel window
		m_avail_size = ImGui::GetContentRegionAvail();
		m_imageSize = m_avail_size;

		// start position of viewport relative to the main glfw window
		m_viewportPosX =int(m_windowPos.x + (m_windowSize.x - m_avail_size.x));
		m_viewportPosY =int(m_windowPos.y + (m_windowSize.y - m_avail_size.y));


		unsigned int vSizeX = (unsigned int)m_imageSize.x;
		if (m_imageSize.x <= 0) {
			vSizeX = 1;
		}

		unsigned int vSizeY = (unsigned int)m_imageSize.y;
		if (m_imageSize.y <= 0) {
			vSizeY = 1;
		}

		m_engineInstance.GetWindow().SetViewportData(vSizeX, vSizeY, m_viewportPosX, m_viewportPosY);

		// Display Rendered Frame
		ImGui::Image((void*)m_engineInstance.GetRenderer().GetRenderTextureId(), m_imageSize, ImVec2(0, 1), ImVec2(1, 0));


		if (ImGui::IsWindowHovered())
		{
			ImGui::CaptureMouseFromApp(false);
			ImGui::CaptureKeyboardFromApp(false);
		}


		ImGui::End();
	}
}