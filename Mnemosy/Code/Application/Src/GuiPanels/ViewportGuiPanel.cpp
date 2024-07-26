#include "Include/GuiPanels/ViewportGuiPanel.h"

#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Window.h"
#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Renderer.h"

//#include "Include/Core/FileDirectories.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS


namespace mnemosy::gui
{
	ViewportGuiPanel::ViewportGuiPanel()
		: m_engineInstance{ MnemosyEngine::GetInstance() }
	{
		panelName = "Viewport";
		panelType = MNSY_GUI_PANEL_VIEWPORT;
		//m_currentSelectedSkybox = MnemosyEngine::GetInstance().GetSkyboxAssetRegistry().GetPositionByName("Market");
	}

	void ViewportGuiPanel::Draw()
	{
		if (!showPanel)
			return;




		ImGui::Begin(panelName, &showPanel, ImGuiWindowFlags_MenuBar);


		//ImGui::BeginMenuBar();
		if (ImGui::BeginMenuBar()) {

			MnemosyEngine& engine = MnemosyEngine::GetInstance();

			graphics::Scene& scene = engine.GetScene();
		

			// Quick select mesh
			{
				graphics::RenderMesh& renderMesh = scene.GetMesh();

				const char* previewMesh_List[7] = { "Custom","Default","Cube","Plane","Sphere","Cylinder","Suzanne" }; // they need to be ordered the same as in lightType Enum in light class

				int previewMesh_Current = (int)scene.GetCurrentPreviewMesh();


				ImGui::Text("Mesh: ");
				ImGui::SetNextItemWidth(120.0f);
				ImGui::Combo(" ##ViewportMesh", &previewMesh_Current, previewMesh_List, IM_ARRAYSIZE(previewMesh_List));

				if ((int)scene.GetCurrentPreviewMesh() != previewMesh_Current)
				{
					if (previewMesh_Current == 0)
						scene.SetPreviewMesh(graphics::PreviewMesh::Custom);
					else if (previewMesh_Current == 1)
						scene.SetPreviewMesh(graphics::PreviewMesh::Default);
					else if (previewMesh_Current == 2)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cube);
					else if (previewMesh_Current == 3)
						scene.SetPreviewMesh(graphics::PreviewMesh::Plane);
					else if (previewMesh_Current == 4)
						scene.SetPreviewMesh(graphics::PreviewMesh::Sphere);
					else if (previewMesh_Current == 5)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cylinder);
					else if (previewMesh_Current == 6)
						scene.SetPreviewMesh(graphics::PreviewMesh::Suzanne);
				}

			}

			// Quick Select Skybox

			graphics::Skybox& skybox = scene.GetSkybox();
			mnemosy::systems::SkyboxAssetRegistry& skyboxRegistry = engine.GetSkyboxAssetRegistry();

			// -- Skybox Selection Menu
			bool assetsInRegistry = !skyboxRegistry.GetVectorOfNames().empty();
			if (assetsInRegistry) // if there are no assets in the internal vector this will crash
			{

				int current = skyboxRegistry.GetCurrentSelected();

				const char* combo_preview_value = skyboxRegistry.GetVectorOfNames()[current].c_str();
				int previousSelected = current;

				ImGui::Text("Skybox: ");
				ImGui::SetNextItemWidth(200.0f);
				if (ImGui::BeginCombo(" ##ViewportSkybox", combo_preview_value, 0))
				{
					for (int n = 0; n < skyboxRegistry.GetVectorOfNames().size(); n++)
					{
						const bool is_selected = (current == n);
						if (ImGui::Selectable(skyboxRegistry.GetVectorOfNames()[n].c_str(), is_selected))
							current = n;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (current != previousSelected)  // Selection has happend
				{
					skybox.LoadPreviewSkybox(skyboxRegistry.GetVectorOfNames()[current]);
					engine.GetRenderer().SetShaderSkyboxUniforms(skybox);
				}
			}




			ImGui::EndMenuBar();
		}



		DrawViewport();

		ImGui::End();
	}




	void ViewportGuiPanel::DrawViewport() {


		// width and height of the entrire imGui window including top bar and left padding
		m_windowSize = ImGui::GetWindowSize();
		// start position relative to the main glfw window
		m_windowPos = ImGui::GetWindowPos();
		// available size inside the window  use this to define the width and height of the texture image
		// if i subtract this from GetWindowSize() i should get the start coordinates from the image relative to the panel window
		m_avail_size = ImGui::GetContentRegionAvail();
		m_imageSize = m_avail_size;

		// start position of viewport relative to the main glfw window
		m_viewportPosX = int(m_windowPos.x + (m_windowSize.x - m_avail_size.x));
		m_viewportPosY = int(m_windowPos.y + (m_windowSize.y - m_avail_size.y));


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


		if (ImGui::IsWindowHovered() && ImGui::IsWindowDocked())
		{

			ImGui::CaptureMouseFromApp(false);

			ImGui::CaptureKeyboardFromApp(false);

		}


	}
}