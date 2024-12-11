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


#ifdef MNEMOSY_PLATFORM_WINDOWS
	#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

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


		// Library settings
		{
			ImGui::SeparatorText("Library Directory");
			core::FileDirectories& fd = engine.GetFileDirectories();


			std::string currentLibraryDirectory = fd.GetLibraryDirectoryPath().generic_string();
			ImGui::Text("Path: %s", currentLibraryDirectory.c_str());

			if (ImGui::Button("Select Folder...")) {

				// check if the current library folder contains any data
				if (fd.ContainsUserData()) {
					// open popup modal..
					m_openChangeDirectoryModal = true;
				}
				else { // user directory does not contain anything so we can savely set a new path
#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
					if (!directoryPath.empty()) {


						fd.SetNewUserLibraryDirectory(std::filesystem::directory_entry(directoryPath), false, false);
						engine.GetMaterialLibraryRegistry().SaveUserDirectoriesData();

					}
					else {
						MNEMOSY_ERROR("You didnt select a valid folder path");
					}
#endif
				}

			}
			//static bool popModal = false;


			if (m_openChangeDirectoryModal) {

				m_openChangeDirectoryModal = false; // to make sure its only called once
				m_changeDirectoryModelState = true;
				ImGui::OpenPopup("Change Library Directory");
			}


			if (ImGui::BeginPopupModal("Change Library Directory", &m_changeDirectoryModelState, ImGuiWindowFlags_AlwaysAutoResize)) {

				ImGui::Text("The current library directory has some files in it. \nDo you want to copy all contents over to the new directory? ");

				ImGui::Spacing();

				if (ImGui::Button("No Delete All!", ImVec2(200, 0))) {

#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
					if (!directoryPath.empty()) {
						fd.SetNewUserLibraryDirectory(std::filesystem::directory_entry(directoryPath), false, true);
						// delete all material and directory data from material registry
						systems::MaterialLibraryRegistry& registry = MnemosyEngine::GetInstance().GetMaterialLibraryRegistry();
						registry.ClearUserMaterialsAndFolders();

					}
					else {
						MNEMOSY_ERROR("You didnt select a valid folder path");
					}
#endif

					m_changeDirectoryModelState = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();

				if (ImGui::Button("Yes Copy!", ImVec2(200, 0))) {

#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string directoryPath = mnemosy::core::FileDialogs::SelectFolder("");
					if (!directoryPath.empty()) {
						fd.SetNewUserLibraryDirectory(std::filesystem::directory_entry(directoryPath), true, true);
						engine.GetMaterialLibraryRegistry().SaveUserDirectoriesData();
					}
					else {
						MNEMOSY_ERROR("You didnt select a valid folder path");
					}
#endif

					m_changeDirectoryModelState = false;
					ImGui::CloseCurrentPopup();
				}


				ImGui::Spacing();

				if (ImGui::Button("Cancel", ImVec2(150, 0))) {

					m_changeDirectoryModelState = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}



		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();


		// --- Render Mesh settings
		if (ImGui::TreeNode("Mesh Settings"))
		{
			graphics::RenderMesh& renderMesh = scene.GetMesh();

			//ImGui::Text("Mesh Settings");
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

#ifdef MNEMOSY_PLATFORM_WINDOWS
						std::string filepath = mnemosy::core::FileDialogs::OpenFile("FBX (*.fbx)\0*.fbx\0 Obj (*.obj)\0*.obj\0");

						if (!filepath.empty())
						{
							renderMesh.LoadMesh(filepath.c_str());
						}
#endif
					}
				}

			}

			float meshPos[3] = { 1.0f,1.0f,1.0f };
			meshPos[0] = renderMesh.transform.GetPosition().x;
			meshPos[1] = renderMesh.transform.GetPosition().y;
			meshPos[2] = renderMesh.transform.GetPosition().z;
			ImGui::DragFloat3("Mesh Position", (float*)meshPos, 0.1f, -100, 100, "%0.1f");
			glm::vec3 newMeshPos = glm::vec3(meshPos[0], meshPos[1], meshPos[2]);
			renderMesh.transform.SetPosition(newMeshPos);

			float meshRot[3] = { 1.0f,1.0f,1.0f };
			meshRot[0] = renderMesh.transform.GetRotationEulerAngles().x;
			meshRot[1] = renderMesh.transform.GetRotationEulerAngles().y;
			meshRot[2] = renderMesh.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Mesh Rotation", (float*)meshRot, 0.1f, -360.0f, 360.0f, "%0.1f");
			glm::vec3 newMeshRot = glm::vec3(meshRot[0], meshRot[1], meshRot[2]);
			renderMesh.transform.SetRotationEulerAngles(newMeshRot);


			float meshScale[3] = { 1.0f,1.0f,1.0f };
			meshScale[0] = renderMesh.transform.GetScale().x;
			meshScale[1] = renderMesh.transform.GetScale().y;
			meshScale[2] = renderMesh.transform.GetScale().z;
			ImGui::DragFloat3("Mesh Scale", (float*)meshScale, 0.01f, 0.000001f, 10.0f, "%0.4f");
			glm::vec3 newMeshScale = glm::vec3(meshScale[0], meshScale[1], meshScale[2]);
			renderMesh.transform.SetScale(newMeshScale);




			ImGui::TreePop();
		}

		// --- Skybox Settings
		if (ImGui::TreeNode("Background Settings"))
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


			ImGui::SliderFloat("Opacity", &scene.userSceneSettings.background_opacity, 0.0f, 1.0f, "%.4f");
			
			ImGui::ColorEdit3("Background Color", (float*)&scene.userSceneSettings.background_color_r); // hacky, may fail
			ImGui::SliderFloat("Gradient", &scene.userSceneSettings.background_gradientOpacity, 0.0f, 1.0f, "%.4f");
			ImGui::SliderFloat("Blur Radius", &scene.userSceneSettings.background_blurRadius, 0.0f, 2.0f, "%.5f");
			//ImGui::SliderInt("Blur Steps", &scene.userSceneSettings.background_blurSteps, 0, 50);
			
			//ImGui::SliderFloat("Background Rotation", &scene.userSceneSettings.background_rotation, 0.0f, 6.28f, "%.4f");
			ImGui::Spacing();

			ImGui::SliderFloat("Post Exposure", &scene.userSceneSettings.globalExposure, -8.0f, 8.0f, "%.4f");

			// we shouldn't do this every frame
			renderer.SetShaderSkyboxUniforms(scene.userSceneSettings,skybox);

			ImGui::TreePop();
		}

		// --- Light Settings
		if (ImGui::TreeNode("Light Settings"))
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
			ImGui::DragFloat3("Position", (float*)pos, 0.1f, -100, 100, "%0.1f");
			glm::vec3 newPos = glm::vec3(pos[0], pos[1], pos[2]);
			light.transform.SetPosition(newPos);


			float rot[3] = { 1.0f,1.0f,1.0f };
			rot[0] = light.transform.GetRotationEulerAngles().x;
			rot[1] = light.transform.GetRotationEulerAngles().y;
			rot[2] = light.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Rotation", (float*)rot, 0.1f, -360.0f, 360.0f, "%0.1f");
			glm::vec3 newRot = glm::vec3(rot[0], rot[1], rot[2]);
			light.transform.SetRotationEulerAngles(newRot);

			ImGui::DragFloat("Strength",&light.strength,1.0f,0.0f,1000.0f,"%.1f");
			ImGui::SliderFloat("Falloff",&light.falloff,0.01f,5.0f,"%.1f");
			ImGui::ColorEdit3("Color", (float*)&light.color);

			renderer.SetPbrShaderLightUniforms(light);

			ImGui::TreePop();
		}


		// --- Render Settings
		if (ImGui::TreeNode("Render Settings"))
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


		ImGui::End();
	}

} // !mnemosy::gui
