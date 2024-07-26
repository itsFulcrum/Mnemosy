#include "Include/GuiPanels/SceneSettingsGuiPanel.h"
#include "Include/Core/Log.h"

#include "Include/Application.h"
#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"

#include "Include/Graphics/Renderer.h"
#include "External/ImGui/imgui.h"
#include "Include/MnemosyConfig.h"
#ifdef MNEMOSY_PLATFORM_WINDOWS
	#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

#include <glm/glm.hpp>

namespace mnemosy::gui
{
	SceneSettingsGuiPanel::SceneSettingsGuiPanel()
	{
		panelName = "Scene Settings";
		panelType = MNSY_GUI_PANEL_SCENE_SETTINGS;
		m_currentSelectedSkybox = MnemosyEngine::GetInstance().GetSkyboxAssetRegistry().GetPositionByName("Market");
	}

	void SceneSettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		MnemosyEngine& engine = ENGINE_INSTANCE();
		graphics::Scene& scene = engine.GetScene();
		graphics::Renderer& renderer = engine.GetRenderer();

		ImGui::Begin(panelName, &showPanel);
		
		// --- Render Mesh settings
		if (ImGui::TreeNode("Mesh Settings"))
		{
			graphics::RenderMesh& renderMesh = scene.GetMesh();
			
			//ImGui::Text("Mesh Settings");
			{
				const char* previewMesh_List[7] = { "Custom","Default","Cube","Plane","Sphere","Cylinder","Suzanne"}; // they need to be ordered the same as in lightType Enum in light class
				int previewMesh_Current = (int)scene.GetCurrentPreviewMesh();
				ImGui::Combo("Preview Mesh", &previewMesh_Current, previewMesh_List, IM_ARRAYSIZE(previewMesh_List));

				if ((int)scene.GetCurrentPreviewMesh() != previewMesh_Current)
				{
					if (previewMesh_Current == 0)
						scene.SetPreviewMesh(graphics::PreviewMesh::Custom);
					else if(previewMesh_Current == 1)
						scene.SetPreviewMesh(graphics::PreviewMesh::Default);
					else if(previewMesh_Current == 2)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cube);
					else if(previewMesh_Current == 3)
						scene.SetPreviewMesh(graphics::PreviewMesh::Plane);
					else if(previewMesh_Current == 4)
						scene.SetPreviewMesh(graphics::PreviewMesh::Sphere);
					else if(previewMesh_Current == 5)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cylinder);
					else if(previewMesh_Current == 6)
						scene.SetPreviewMesh(graphics::PreviewMesh::Suzanne);
				}

				if (previewMesh_Current == 0)
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
		if (ImGui::TreeNode("Skybox Settings"))
		{
			graphics::Skybox& skybox = scene.GetSkybox();
			mnemosy::systems::SkyboxAssetRegistry& skyboxRegistry = engine.GetSkyboxAssetRegistry();
			
			// -- Skybox Selection Menu
			bool assetsInRegistry = !skyboxRegistry.GetVectorOfNames().empty();
			if (assetsInRegistry) // if there are no assets in the internal vector this will crash
			{
				const char* combo_preview_value = skyboxRegistry.GetVectorOfNames()[m_currentSelectedSkybox].c_str();
				int previousSelected = m_currentSelectedSkybox;
				if (ImGui::BeginCombo("Preview Skyboxes", combo_preview_value, 0))
				{
					for (int n = 0; n < skyboxRegistry.GetVectorOfNames().size(); n++)
					{
						const bool is_selected = (m_currentSelectedSkybox == n);
						if (ImGui::Selectable(skyboxRegistry.GetVectorOfNames()[n].c_str(), is_selected))
							m_currentSelectedSkybox = n;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (m_currentSelectedSkybox != previousSelected)  // Selection has happend
				{
					skybox.LoadPreviewSkybox(skyboxRegistry.GetVectorOfNames()[m_currentSelectedSkybox]);
				}
			}
			
			if (ImGui::Button("Load from file..."))
			{
				if (!m_saveSkyboxPermanentlyUponLoad)
				{
					#ifdef MNEMOSY_PLATFORM_WINDOWS
						std::string filepath =  mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");
						if (!filepath.empty())
						{
							skybox.AssignSkyboxTexture(filepath.c_str(),"NoNameNeeded", 1024, m_saveSkyboxPermanentlyUponLoad);
						}
					#endif
				}
				else
				{
					ImGui::OpenPopup("Save Skybox Texture");
				}
			}
			ImGui::SameLine();
			ImGui::Checkbox("Save Permanently", &m_saveSkyboxPermanentlyUponLoad);

			// -- Loading new Skybox permanently
			if (m_saveSkyboxPermanentlyUponLoad) // called with button "Load from file..."
			{
				bool saveSkyboxModelOpen = true;
				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopupModal("Save Skybox Texture", &saveSkyboxModelOpen, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::TextWrapped("Please provide a unique name and select a resolution \nSo Mnemosy can save the skybox to its internal registry.");
					ImGui::Separator();

					// texfield for name 
					static char skyboxName[32] = "Unique Name";
					ImGui::InputText("##edit", skyboxName, IM_ARRAYSIZE(skyboxName));

					// selection for resolution


					const char* skyboxResolutions_List[4] = { "4096","2048","1024","512"}; // they need to be ordered the same as in lightType Enum in light class
					
					ImGui::TextWrapped("Higher Resolutions take longer to process");
					static int resolution_Selected = 2;
					ImGui::Combo("Resolution", &resolution_Selected, skyboxResolutions_List, IM_ARRAYSIZE(skyboxResolutions_List));


					if (ImGui::Button("Load Skybox Image"))
					{
						// chack if the name string is empty TODO: make this some kind of popup
						bool hasName = true;
						if (skyboxName[0] == '\0')
						{
							hasName = false;
							MNEMOSY_ERROR("You must provide a name to save a skybox permanently so Mnemosy can save it internaly");	
						}
						if (hasName)
						{
							// check if the name already exists TODO: Make this into some pupop
							if (skyboxRegistry.CheckIfExists(skyboxName))
							{
								MNEMOSY_ERROR("A Skybox with the name {} already exists. Names must be unique", skyboxName);
							}
							else
							{
								// get resolution
								unsigned int skyboxExportResolution = 4096;
								if (resolution_Selected == 1)
									skyboxExportResolution = 2048;
								else if (resolution_Selected == 2)
									skyboxExportResolution = 1024;
								else if (resolution_Selected == 3)
									skyboxExportResolution = 512;

								//MNEMOSY_TRACE("Text From Input: {} ", skyboxName);
								//MNEMOSY_TRACE("ResolutionSelected: {} ", skyboxExportResolution);
								// open filebrowser to load image and load it using skybox.AssignSkyboxTexture(filepath.c_str(),1024, m_saveSkyboxPermanentlyUponLoad);

								#ifdef MNEMOSY_PLATFORM_WINDOWS
									std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

									if (!filepath.empty())
									{
										// Function needs to 
										bool success = skybox.AssignSkyboxTexture(filepath.c_str(), skyboxName, skyboxExportResolution, m_saveSkyboxPermanentlyUponLoad);
										if(success)
											m_currentSelectedSkybox = (int)skyboxRegistry.GetVectorOfNames().size() -1;
									}
								#endif
							}
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();

					if (ImGui::Button("Cancel", ImVec2(120, 0))) 
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
			} // !m_saveSkyboxPermanentlyUponLoad
			
			// Removing skybox permanently
			static bool removePermanentlyModelOpen = false;
			if (ImGui::Button("Remove Permanently"))
			{
				removePermanentlyModelOpen = true;
				ImGui::OpenPopup("Remove Skybox Permanently");
			}

			if (ImGui::BeginPopupModal("Remove Skybox Permanently", &removePermanentlyModelOpen, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Select skybox to delete permanently");
				ImGui::Separator();


				// get list of skyboxs available to remove
				static int selectedToRemove = 0;
				const char* combo_preview_value = skyboxRegistry.GetVectorOfNames()[selectedToRemove].c_str();// items[item_current_idx];
				if (ImGui::BeginCombo("Skyboxes", combo_preview_value, 0))
				{
					for (int n = 0; n < skyboxRegistry.GetVectorOfNames().size(); n++)
					{
						const bool is_selected = (selectedToRemove == n);
						if (ImGui::Selectable(skyboxRegistry.GetVectorOfNames()[n].c_str(), is_selected))
							selectedToRemove = n;


						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}


				std::string nameOfSkyboxToDelete = skyboxRegistry.GetVectorOfNames()[selectedToRemove];

				if (ImGui::Button("Delete", ImVec2(120, 0)))
				{
					// Dont allow to delete the last one
					bool canRemove = true;
					if (nameOfSkyboxToDelete == "Market")
					{
						canRemove = false;
						MNEMOSY_ERROR("You are not allowed to remove the default skybox Market")
					}

					if (canRemove)
					{

						MNEMOSY_WARN("Removing Skybox {}", nameOfSkyboxToDelete);
						skyboxRegistry.RemoveEntry(nameOfSkyboxToDelete);

						// TODO: delete actual files from disk -> shoudl be part of removeEntry probaly

						// set selected skybox to first one 
						selectedToRemove = 0;
						std::string firstSkyboxName = skyboxRegistry.GetVectorOfNames()[0];
						skybox.LoadPreviewSkybox(firstSkyboxName);
						m_currentSelectedSkybox = 0;
					}


					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();

				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					ImGui::CloseCurrentPopup();


				ImGui::EndPopup();
			}



			ImGui::SliderFloat("Exposure", &skybox.exposure, -5.0f, 5.0f, "%.4f");
			ImGui::SliderFloat("Rotation", &skybox.rotation, 0.0f, 6.28f, "%.4f");
			
			ImGui::ColorEdit3("Background Color", (float*)&skybox.backgroundColor);
			ImGui::SliderFloat("Opacity", &skybox.opacity, 0.0f, 1.0f, "%.4f");
			ImGui::SliderFloat("Gradient", &skybox.gradientOpacity, 0.0f, 1.0f, "%.4f");
			ImGui::ColorEdit3("Color Tint", (float*)&skybox.colorTint);
			ImGui::SliderFloat("Blur Radius", &skybox.blurRadius, 0.0f, 2.0f, "%.5f");
			ImGui::SliderInt("Blur Steps", &skybox.blurSteps, 0, 50);

			renderer.SetShaderSkyboxUniforms(skybox);

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

		//// --- Camera Settings
		//if (ImGui::TreeNode("Camera Settings"))
		//{
		//	graphics::Camera& camera = scene.GetCamera();

		//	ImGui::SliderFloat("Field of View", &camera.settings.fov, 1.0f, 160.0f, "%.1f");

		//	ImGui::DragFloat("Near Clip", &camera.settings.nearClip, 0.1f, 0.00001f, 100.0f, "%0.4f");
		//	ImGui::DragFloat("Far Clip", &camera.settings.farClip, 0.1f, 0.02f, 5000.0f, "%0.4f");


		//	ImGui::TreePop();
		//}

		// --- Post Processing Settings
		if (ImGui::TreeNode("Post processing"))
		{

			ImGui::Text("Here will go Post Processing settings");

			ImGui::TreePop();
		}


		ImGui::End();
	}

} // !mnemosy::gui