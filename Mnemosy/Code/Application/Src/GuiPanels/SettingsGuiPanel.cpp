#include "Include/GuiPanels/SettingsGuiPanel.h"

#include "Include/MnemosyConfig.h"
#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/Clock.h"
#include "Include/ApplicationConfig.h"


#include "Include/Core/FileDirectories.h"

#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/FolderTreeNode.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Renderer.h"


#include "ImGui/imgui.h"

#include "Include/Core/Utils/PlatfromUtils_Windows.h"


#include <filesystem>
#include <glm/glm.hpp>

namespace mnemosy::gui
{
	SettingsGuiPanel::SettingsGuiPanel()
	{
		panelName = "Settings";
		panelType = MNSY_GUI_PANEL_SETTINGS;
	}



	void SettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		MnemosyEngine& engine = MnemosyEngine::GetInstance();;
		graphics::Scene& scene = engine.GetScene();
		graphics::Renderer& renderer = engine.GetRenderer();

		ImGui::Begin(panelName, &showPanel);


		ImGui::Spacing();
		ImGui::Spacing();



		// --- Background Settings
		if (ImGui::TreeNode("Background"))
		{
			graphics::Skybox& skybox = scene.GetSkybox();
			mnemosy::systems::SkyboxAssetRegistry& skyReg = engine.GetSkyboxAssetRegistry();

			// -- Skybox Selection Menu

			const std::vector<std::string>& list = skyReg.GetEntryList();

			if (!list.empty())
			{
				uint16_t current_id = skyReg.GetCurrentSelectedID();


				const char* combo_preview_value = list[current_id].c_str();
				

				bool removeEntry = false;
				uint16_t removeId = 0;
				if (ImGui::BeginCombo("Preview Skyboxes", combo_preview_value, 0))
				{
					for (uint16_t n = 0; n < list.size(); n++)
					{
						const bool is_selected = (current_id == n);
						if (ImGui::Selectable(list[n].c_str(), is_selected)) {
							current_id = n;
						}

						if (ImGui::BeginPopupContextItem()) {
							
							if (ImGui::Selectable("Remove From Quick Select")) {
								
								removeEntry = true;
								removeId = n;
							}
							ImGui::EndPopup();
						}


						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (removeEntry) {

					if (removeId == skyReg.GetCurrentSelectedID()) {

						scene.SetSkybox(skyReg.LoadPreviewSkybox(0, true));
						engine.GetRenderer().SetShaderSkyboxUniforms(scene.userSceneSettings, scene.GetSkybox());
					}
					skyReg.RemoveEntry(removeId);				
				}
				else if (current_id != skyReg.GetCurrentSelectedID())  // Selection has happend
				{

					if (MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().UserEntrySelected()) {
						if (MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().ActiveLibEntry_Get()->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

							// if the active libEntry is of type skybox we must select default material because when switching preview skybox because 
							// otherwise it messes up thumbnail rendering of the libEntry when switching to a new one and it also overrides material data 
							// since when saving we grab the data from the aktivly loaded skybox which has changed by choosing a new preview skybox. 

							MnemosyEngine::GetInstance().GetMaterialLibraryRegistry().SetDefaultMaterial();
						}
					}

					scene.SetSkybox(skyReg.LoadPreviewSkybox(current_id,true));
					engine.GetRenderer().SetShaderSkyboxUniforms(scene.userSceneSettings, scene.GetSkybox());
				}
			}

			skybox = scene.GetSkybox(); // we have to set this again because we may have changed it in the quick selection by removing an entry


			ImGui::SliderFloat("Opacity", &scene.userSceneSettings.background_opacity, 0.0f, 1.0f, "%.2f");
			
			ImGui::ColorEdit3("Background Color", (float*)&scene.userSceneSettings.background_color_r); // hacky, may fail
			ImGui::SliderFloat("Gradient", &scene.userSceneSettings.background_gradientOpacity, 0.0f, 1.0f, "%.2f");
			ImGui::SliderFloat("Blur", &scene.userSceneSettings.background_blurRadius, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderInt("Blur Steps", &scene.userSceneSettings.background_blurSteps, 0, 50);
			
			//ImGui::SliderFloat("Background Rotation", &scene.userSceneSettings.background_rotation, 0.0f, 6.28f, "%.4f");
			
			ImGui::DragFloat("Post Exposure", &scene.userSceneSettings.globalExposure, 0.005f, -8.0f, 8.0f, "%.3f");

			// we shouldn't do this every frame
			renderer.SetShaderSkyboxUniforms(scene.userSceneSettings,skybox);

			ImGui::TreePop();
		}
		
		ImGui::Separator();

		// --- Light Settings
		if (ImGui::TreeNode("Light"))
		{
			graphics::Light& light = scene.GetLight();

			//ImGui::Text("Light Settings");
			const char* lightTypes_List[] = { "Directional", "Point" }; // they need to be ordered the same as in lightType Enum in light class
			static int lightType_current = light.GetLightTypeAsInt();

			ImGui::Combo("Light Type", &lightType_current, lightTypes_List, IM_ARRAYSIZE(lightTypes_List));

			if (light.GetLightTypeAsInt() != lightType_current)
			{
				light.SetType((graphics::LightType)lightType_current);
			}


			float pos[3] = { 1.0f,1.0f,1.0f };
			pos[0] = light.transform.GetPosition().x;
			pos[1] = light.transform.GetPosition().y;
			pos[2] = light.transform.GetPosition().z;
			ImGui::DragFloat3("Position", (float*)pos, 0.1f, -100, 100, "%0.2f");
			glm::vec3 newPos = glm::vec3(pos[0], pos[1], pos[2]);
			light.transform.SetPosition(newPos);


			float rot[3] = { 1.0f,1.0f,1.0f };
			rot[0] = light.transform.GetRotationEulerAngles().x;
			rot[1] = light.transform.GetRotationEulerAngles().y;
			rot[2] = light.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Rotation", (float*)rot, 0.1f, -360.0f, 360.0f, "%0.2f");
			glm::vec3 newRot = glm::vec3(rot[0], rot[1], rot[2]);
			light.transform.SetRotationEulerAngles(newRot);

			ImGui::ColorEdit3("Color", (float*)&light.color);
			ImGui::DragFloat("Strength",&light.strength,0.02f,0.0f,1000.0f,"%.3f");
			ImGui::DragFloat("Falloff",&light.falloff,0.002f,0.05f,5.0f,"%.3f");

			renderer.SetPbrShaderLightUniforms(light);

			ImGui::TreePop();
		}

		ImGui::Separator();

		// --- Render Settings
		if (ImGui::TreeNode("Render"))
		{
			ImGui::Spacing();


#ifdef mnemosy_gui_showFps

			core::Clock& clock = engine.GetClock();
			
			ImGui::SeparatorText("Debug Info");
			// show fps and frametime in ms
			int fps = clock.GetFPS();
			float deltaSeconds = clock.GetFrameTime();
			ImGui::Text("FPS: %d", fps);
			ImGui::Text("FrameTime: %f ms", deltaSeconds);

#endif // mnemosy_gui_showDebugInfo

			// --- Render Settings
			{

				graphics::Renderer& renderer = engine.GetRenderer();

				// MSAA
				{
					const char* MSAA_Settings[5] = { "OFF","2X","4X","8X","16X" }; // they need to be ordered the same as in renderer MSAAsamples Enum
					int previewMSAA_Current = renderer.GetMSAAEnumAsInt();
					ImGui::Combo("MSAA", &previewMSAA_Current, MSAA_Settings, IM_ARRAYSIZE(MSAA_Settings));
					if (previewMSAA_Current != renderer.GetMSAAEnumAsInt())
					{
						renderer.SetMSAASamples((graphics::MSAAsamples)previewMSAA_Current);
					}

				}

				// thumbnail res
				{
					const char* Thumbnail_Resolutions[4] = { "64","128","256","512"}; // they need to be ordered the same as in renderer MSAAsamples Enum

					int current_thumb_res = (int)renderer.GetThumbnailResolutionEnum();
					ImGui::Combo("Thumbnail Resolution", &current_thumb_res, Thumbnail_Resolutions, IM_ARRAYSIZE(Thumbnail_Resolutions));
					if (current_thumb_res != (int)renderer.GetThumbnailResolutionEnum()) {
						renderer.SetThumbnailResolution((graphics::ThumbnailResolution)current_thumb_res);
					}

				}



			}

			ImGui::TreePop();
		}
		
		ImGui::Separator();
		
		// --- Render Mesh settings
		if (ImGui::TreeNode("Mesh"))
		{
			graphics::RenderMesh& renderMesh = scene.GetMesh();

			// Mesh Selection
			{
				const char* previewMesh_List[8] = { "Custom","Default","Cube","Plane","Sphere","Cylinder","Suzanne","Fabric"}; // they need to be ordered the same as in lightType Enum in light class
				int previewMesh_Current = (int)scene.GetCurrentPreviewMesh();
				ImGui::Combo("Preview Mesh", &previewMesh_Current, previewMesh_List, IM_ARRAYSIZE(previewMesh_List));

				if ((int)scene.GetCurrentPreviewMesh() != previewMesh_Current)
				{
					scene.SetPreviewMesh((graphics::PreviewMesh)previewMesh_Current);
				}

				if ((graphics::PreviewMesh)previewMesh_Current == graphics::PreviewMesh::Custom)
				{
					if (ImGui::Button("Load Mesh..."))
					{
						std::filesystem::path filepath = mnemosy::core::FileDialogs::OpenFile("FBX (*.fbx)\0*.fbx\0 Obj (*.obj)\0*.obj\0");

						if (!filepath.empty())
						{
							renderMesh.LoadMesh(filepath.generic_string().c_str());
						}
					}
				}

			}

			float meshPos[3] = { 1.0f,1.0f,1.0f };
			meshPos[0] = renderMesh.transform.GetPosition().x;
			meshPos[1] = renderMesh.transform.GetPosition().y;
			meshPos[2] = renderMesh.transform.GetPosition().z;
			ImGui::DragFloat3("Position##Mesh", (float*)meshPos, 0.1f, -100, 100, "%0.2f");
			glm::vec3 newMeshPos = glm::vec3(meshPos[0], meshPos[1], meshPos[2]);
			renderMesh.transform.SetPosition(newMeshPos);

			float meshRot[3] = { 1.0f,1.0f,1.0f };
			meshRot[0] = renderMesh.transform.GetRotationEulerAngles().x;
			meshRot[1] = renderMesh.transform.GetRotationEulerAngles().y;
			meshRot[2] = renderMesh.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Rotation ##Mesh", (float*)meshRot, 0.1f, -360.0f, 360.0f, "%0.2f");
			glm::vec3 newMeshRot = glm::vec3(meshRot[0], meshRot[1], meshRot[2]);
			renderMesh.transform.SetRotationEulerAngles(newMeshRot);


			float meshScale[3] = { 1.0f,1.0f,1.0f };
			meshScale[0] = renderMesh.transform.GetScale().x;
			meshScale[1] = renderMesh.transform.GetScale().y;
			meshScale[2] = renderMesh.transform.GetScale().z;
			ImGui::DragFloat3("Scale ##Scale", (float*)meshScale, 0.01f, 0.000001f, 10.0f, "%0.2f");
			glm::vec3 newMeshScale = glm::vec3(meshScale[0], meshScale[1], meshScale[2]);
			renderMesh.transform.SetScale(newMeshScale);




			ImGui::TreePop();
		}


		ImGui::End();
	}

} // !mnemosy::gui
