#include "Include/GuiPanels/MaterialEditorGuiPanel.h"

#include "Include/Application.h"
#include "Include/ApplicationConfig.h"
#include "Include/MnemosyEngine.h"

#include "Include/Core/FileDirectories.h"
#include "Include/Core/Log.h"
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#include "Include/Core/Utils/DropHandler_Windows.h"

#include "Include/Systems/Input/InputSystem.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Systems/ExportManager.h"
#include "Include/Systems/FolderTreeNode.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Texture.h"

#include "External/ImGui/imgui.h"

#include <glm/glm.hpp>
#include <filesystem>


namespace mnemosy::gui
{	
	MaterialEditorGuiPanel::MaterialEditorGuiPanel()
		: m_materialRegistry{ ENGINE_INSTANCE().GetMaterialLibraryRegistry() }
	{
		panelName = "Material Editor";

		m_onFileDropInput_callback_id = MnemosyEngine::GetInstance().GetInputSystem().REGISTER_DROP_INPUT(&MaterialEditorGuiPanel::OnFileDropInput);
	}

	MaterialEditorGuiPanel::~MaterialEditorGuiPanel()
	{
		//MNEMOSY_TRACE("MaterialEditorGuiPanel: Destructor");

		MnemosyEngine::GetInstance().GetInputSystem().UnregisterDropInput(m_onFileDropInput_callback_id);
	}

	void MaterialEditorGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		ImGui::Begin(panelName.c_str(), &showPanel);

		m_isPanelHovered = ImGui::IsWindowHovered();

		

		// variables used across entire method
		MnemosyEngine& engineInstance = MnemosyEngine::GetInstance();
		graphics::Material& activeMat = engineInstance.GetScene().GetActiveMaterial();
		ImVec2 buttonSize = ImVec2(120, 0);
		const char* normalMapFormats[] = { "OpenGl", "DirectX" }; // they need to be ordered the same as in material NormalMapFormat Enum
		const char* exportFormats[] = { "ktx2", "png","tiff"}; // they need to be ordered the same as in ExportManager ExportImageFormats
		fs::path libDir = engineInstance.GetFileDirectories().GetLibraryDirectoryPath();
		systems::ExportManager& exportManager = engineInstance.GetExportManager();


		// Check and see if the active material changed
		if (m_materialRegistry.UserMaterialBound()) {
			
			int nowActiveMaterialID = m_materialRegistry.GetActiveMaterialID();

			if (m_currentActiveMaterialID != nowActiveMaterialID) {
				// Active material Changed
				m_currentActiveMaterialID = nowActiveMaterialID;


				m_readyToDragMaterial = false;
			}
		}
		else {
			m_currentActiveMaterialID = -1;
			m_readyToDragMaterial = false;
		}

		

		if (m_isAbedoButtonHovered 
			|| m_isNormalButtonHovered
			|| m_isRoughnessButtonHovered 
			|| m_isMetallicButtonHovered
			|| m_isAmbientOcclusionButtonHovered
			|| m_isEmissionButtonHovered ) {
			engineInstance.GetDropHandler().SetDropTargetActive(true);
		}
		else {
			engineInstance.GetDropHandler().SetDropTargetActive(false);
		}


		std::string displayText = "Material: " + activeMat.Name;
		ImGui::SeparatorText(displayText.c_str());

		if (m_materialRegistry.UserMaterialBound()) {

			

			// DEBUG INFO
#ifdef mnemosy_gui_showDebugInfo
				std::filesystem::path matDataFile = m_materialRegistry.GetActiveMaterialDataFilePath();
				std::string matDataFileString = 
					"Debug Dev info:" 
					"\n	Active Material: " + activeMat.Name + 
					"\n	Active ID: " + std::to_string(m_materialRegistry.GetActiveMaterialID()) + 
					//"\n Folder: " + m_materialRegistry.m_folderNodeOfActiveMaterial.name +
					"\n	DataFilePath: " + matDataFile.generic_string();
				ImGui::Text(matDataFileString.c_str());			
#endif // mnemosy_gui_showDebugInfo
				
			ImGui::Spacing();

			


			


			// Export Settings
			{

				if (ImGui::TreeNode("Export Settings")) {

					int exportImageFormat_current = exportManager.GetExportImageFormatInt();
					ImGui::Combo("Image Format ##Export", &exportImageFormat_current, exportFormats, IM_ARRAYSIZE(exportFormats));
					ImGui::SetItemTooltip("Specify the image format used for exporting");

					// if format changed
					if (exportManager.GetExportImageFormatInt() != exportImageFormat_current) {

						m_readyToDragMaterial = false;
						exportManager.SetExportImageFormatInt(exportImageFormat_current);
					}


					// Normal Map Format
					int exportNormalFormat_current = exportManager.GetNormalMapExportFormatInt();
					ImGui::Combo("Normal Map Format ##Export", &exportNormalFormat_current, normalMapFormats, IM_ARRAYSIZE(normalMapFormats));
					ImGui::SetItemTooltip("Specify the normal map format to export normal maps with");

					// if format changed
					if (exportManager.GetNormalMapExportFormatInt() != exportNormalFormat_current) {
						m_readyToDragMaterial = false;
						exportManager.SetNormalMapExportFormatInt(exportNormalFormat_current);
					} 

					if (ImGui::Button("Export To...",buttonSize)) {

						std::string exportFolder = mnemosy::core::FileDialogs::SelectFolder("");
						if (!exportFolder.empty()) {

							m_materialRegistry.SaveActiveMaterialToFile();
							fs::path materialFolderPath = libDir / fs::path(m_materialRegistry.m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);
							fs::path exportPath = fs::path(exportFolder);
							exportManager.ExportMaterialTextures(exportPath,materialFolderPath, activeMat);
						}
					}

					ImGui::TreePop();
				}

			}// End Export Settings

			// Drag Material 
			{
				std::string dragButtonName = "Drag";
				ImVec2 dragButton = ImVec2(120.0f, 20.0f);
				if (m_readyToDragMaterial) {
					dragButtonName = "Drag Me";
				}
				else {
					dragButtonName = "Prepare Drag";
				}

				if (ImGui::Button(dragButtonName.c_str(), ImVec2(120.0f, 45.0f))) {
					// Export Active Material Texture to a temporary folder so we can drag them.
					// Temp Folder is delete if we close menmosy or when we start it.
					fs::path tempFolder = engineInstance.GetFileDirectories().GetTempExportFolderPath();
					if (!tempFolder.empty()) {
						fs::path materialFolderPath = libDir / fs::path(m_materialRegistry.m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);
						exportManager.ExportMaterialTextures(tempFolder, materialFolderPath, activeMat);
					}

					m_readyToDragMaterial = true;
					m_isDraggingOnce = true;
				}

				if (m_readyToDragMaterial) {

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {

						if (m_isDraggingOnce) {

							engineInstance.GetDropHandler().BeginDrag(exportManager.GetLastExportedFilePaths());
						}
						m_isDraggingOnce = false;
						ImGui::EndDragDropSource();
					}
				}

			} // End Drag Material 




		}

		ImGui::Spacing();
		ImGui::Spacing();
		const char* readImageFormats = "TIF (*.tif)\0*.tif\0 HDR (*.hdr)\0*.hdr\0 PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0 TIFF (*.tiff)\0*.tiff\0";
		ImVec2 buttonSizeLoad = ImVec2(120, 0);
		ImVec2 buttonSizeDelete = ImVec2(80, 0);

		// Albedo Settings
		{	
			ImGui::SeparatorText("Albedo ");
			// Load Texture
			if (ImGui::Button("Load Texture...##Albedo",buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::ALBEDO, filepath);
					m_readyToDragMaterial = false;
				}
			}
			m_isAbedoButtonHovered = ImGui::IsItemHovered();


			bool textureAssigned = activeMat.isAlbedoAssigned();


			// we cant merge this with the below if statement because the button can change the state of the texture assigned bool
			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Albedo", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::ALBEDO);
					textureAssigned = false;
					m_readyToDragMaterial = false;
				}
			}

			if (textureAssigned) {

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetAlbedoTexture().GetWidth()) + "x" + std::to_string(activeMat.GetAlbedoTexture().GetHeight());
				ImGui::Text(resolution.c_str());

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::ALBEDO));
				ImGui::Text(TextureID.c_str());				
#endif // mnemosy_gui_showDebugInfo


			}

			// Disabled widgets if texture is assigned 
			if(textureAssigned)
				ImGui::BeginDisabled();

			ImGui::ColorEdit3("Albedo Color", (float*)&activeMat.Albedo);
			
			if (textureAssigned)
				ImGui::EndDisabled();

		} // End Albedo Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Roughness Settings
		{
			ImGui::SeparatorText("Roughness ");
			bool textureAssigned = activeMat.isRoughnessAssigned();
			
			// Load Texture
			if (ImGui::Button("Load Texture...##Roughness",buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::ROUGHNESS, filepath);
					m_readyToDragMaterial = false;
				}
			}
			m_isRoughnessButtonHovered = ImGui::IsItemHovered();



			if (textureAssigned) {

				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Roughness", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::ROUGHNESS);
					textureAssigned = false;
					m_readyToDragMaterial = false;
				}
			}

			if (textureAssigned) {

				

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetRoughnessTexture().GetWidth()) + "x" + std::to_string(activeMat.GetRoughnessTexture().GetHeight());
				ImGui::Text(resolution.c_str());

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::ROUGHNESS));
				ImGui::Text(TextureID.c_str());				
#endif // mnemosy_gui_showDebugInfo


			}
			
			// Disabled widgets if texture is assigned 
			if (textureAssigned)
				ImGui::BeginDisabled();
			{
				ImGui::SliderFloat("Roughness", &activeMat.Roughness, 0.0f, 1.0f, "%.4f");

			}
			if (textureAssigned)
				ImGui::EndDisabled();

		} // End Roughness Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Normal Settings
		{ 
			ImGui::SeparatorText("Normal ");
			bool textureAssigned = activeMat.isNormalAssigned();

			//  Load Texture
			if (ImGui::Button("Load Texture...##Normal",buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::NORMAL, filepath);
					m_readyToDragMaterial = false;
				}
			}
			m_isNormalButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Normal", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::NORMAL);
					textureAssigned = false;
					m_readyToDragMaterial = false;
				}
			}

			if (textureAssigned) {


				std::string resolution = "Resolution: " + std::to_string(activeMat.GetNormalTexture().GetWidth()) + "x" + std::to_string(activeMat.GetNormalTexture().GetHeight());
				ImGui::Text(resolution.c_str());

				// DEBUG INFO
#ifdef mnemosy_gui_showDebugInfo
					std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::NORMAL));
					ImGui::Text(TextureID.c_str());
#endif // mnemosy_gui_showDebugInfo
			}

			// Disabled widgets if texture is NOT assigned 
			if (!textureAssigned)
				ImGui::BeginDisabled();
			{
				ImGui::DragFloat("Normal Strength", &activeMat.NormalStrength,0.01f, 0.0f, 100.0f, "%.4f");
				
				// NORMAL MAP FORMAT
				
				if (m_materialRegistry.UserMaterialBound()) { // normal convert feature for now not allowed for the default material because it doesnt have a filepath to save the texture to
									
					
					int format_current = activeMat.GetNormalFormatAsInt();

					ImGui::Combo("Normal Map Format", &format_current, normalMapFormats, IM_ARRAYSIZE(normalMapFormats));
					ImGui::SetItemTooltip("Specify the normal map format of the source texture");

					// if format changed
					if (activeMat.GetNormalFormatAsInt() != format_current) {
						fs::path materialFolderPath = libDir / fs::path(m_materialRegistry.m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);
						fs::path normalMapPath = materialFolderPath / fs::path(std::string(activeMat.Name + "_normal.ktx2"));
										
						// Generate inverted normal Texture.
						MnemosyEngine::GetInstance().GetTextureGenerationManager().FlipNormalMap(normalMapPath.generic_string().c_str(), activeMat,true);

						// save normal format to material data file.
						if (format_current == 0) {
							activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGl);
						}
						else if (format_current == 1) {
							activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
						}
						m_materialRegistry.SaveActiveMaterialToFile();

						// Load newly created texture as new normal map 
						activeMat.GetNormalTexture().LoadFromKtx(normalMapPath.generic_string().c_str());
						m_readyToDragMaterial = false;

					}
				}


			}
			if (!textureAssigned)
				ImGui::EndDisabled();

		} // End Normal Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Metallic Settings
		{ 
			ImGui::SeparatorText("Metallic ");
			bool textureAssigned = activeMat.isMetallicAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##Metallic", buttonSizeLoad)) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::METALLIC, filepath);
					m_readyToDragMaterial = false;
				}
			}
			m_isMetallicButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Metallic", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::METALLIC);
					textureAssigned = false;
					m_readyToDragMaterial = false;
				}
			}

			if (textureAssigned) {


				std::string resolution = "Resolution: " + std::to_string(activeMat.GetMetallicTexture().GetWidth()) + "x" + std::to_string(activeMat.GetMetallicTexture().GetHeight());
				ImGui::Text(resolution.c_str());

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::METALLIC));
				ImGui::Text(TextureID.c_str());
#endif // mnemosy_gui_showDebugInfo

			}

			// Disabled widgets if texture is assigned 
			if (textureAssigned)
				ImGui::BeginDisabled();
			{
				ImGui::SliderFloat("Metallic", &activeMat.Metallic, 0.0f, 1.0f, "%.4f");
			}
			if (textureAssigned)
				ImGui::EndDisabled();

		} // End Metallic Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Ambient Occlusion Settings
		{ 
			ImGui::SeparatorText("Abient Occlusion ");
			bool textureAssigned = activeMat.isAoAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##AO", buttonSizeLoad)) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::AMBIENTOCCLUSION, filepath);
					m_readyToDragMaterial = false;
				}
			}
			m_isAmbientOcclusionButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##AO",buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::AMBIENTOCCLUSION);
					textureAssigned = false;
					m_readyToDragMaterial = false;
				}
			}

			if (textureAssigned) {


				std::string resolution = "Resolution: " + std::to_string(activeMat.GetAOTexture().GetWidth()) + "x" + std::to_string(activeMat.GetAOTexture().GetHeight());
				ImGui::Text(resolution.c_str());

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::AMBIENTOCCLUSION));
				ImGui::Text(TextureID.c_str());				
#endif // mnemosy_gui_showDebugInfo

			}
		} // End Ambient Occlusion Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Emission Settings
		{ 
			ImGui::SeparatorText("Emission ");
			bool textureAssigned = activeMat.isEmissiveAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##Emissive", buttonSizeLoad)) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::EMISSION, filepath);
					m_readyToDragMaterial = false;
				}
			}
			m_isEmissionButtonHovered = ImGui::IsItemHovered();


			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Emissive", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::EMISSION);
					textureAssigned = false;
					m_readyToDragMaterial = false;
				}
			}

			if (textureAssigned) {

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetEmissiveTexture().GetWidth()) + "x" + std::to_string(activeMat.GetEmissiveTexture().GetHeight());
				ImGui::Text(resolution.c_str());

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::EMISSION));
				ImGui::Text(TextureID.c_str());				
#endif // mnemosy_gui_showDebugInfo


			}

			// Disabled widgets if texture is assigned 
			if (textureAssigned)
				ImGui::BeginDisabled();
			{
				ImGui::ColorEdit3("Emission", (float*)&activeMat.Emission);
			}
			if (textureAssigned)
				ImGui::EndDisabled();

			ImGui::DragFloat("Emission Strength", &activeMat.EmissionStrength,0.01f, 0.0f, 10000.0f, "%.4f");

		} // End Emission Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// General Settings
		{
			ImGui::SeparatorText("General ");

			float uvTiling[2] = { activeMat.UVTiling.x,activeMat.UVTiling.y };
			if (ImGui::DragFloat2("UV Tiling", (float*)uvTiling, 0.1f, 0.0, 1000, "%0.4f")) {
				activeMat.UVTiling.x = uvTiling[0];
				activeMat.UVTiling.y = uvTiling[1];
			}
		} // End General Settings
		
		ImGui::Spacing();
		ImGui::Spacing();


		if (ImGui::Button("Save Material", buttonSize)) {

			m_materialRegistry.SaveActiveMaterialToFile();
		}


		ImGui::End();

	} // End Draw()

	// Callback when files are droped into mnemosy
	void MaterialEditorGuiPanel::OnFileDropInput(int count, std::vector<std::string>& dropedFilePaths) {
		if (m_isPanelHovered) {

			if (count > 0) {

				if (!dropedFilePaths.empty()) { // just making sure

					// we kinda only want to handle the first one.
					std::string firstPath = dropedFilePaths[0];
					//MNEMOSY_TRACE("MaterialEditorGuiPanel::OnFileDropInput: fist droped File: {}", firstPath);
					fs::directory_entry firstFile = fs::directory_entry(firstPath);

					if (firstFile.exists()) {

						if (firstFile.is_regular_file()) {

							std::string extention = firstFile.path().extension().generic_string();
							if (extention == ".png" || extention == ".tif" || extention == ".tiff" || extention == ".jpg" || extention == ".jpeg") {
								//MNEMOSY_DEBUG("MaterialEditorGuiPanel::OnFileDropInput: FirstFile is Valid File. Filetype: {}",extention);
								// Valid texture file

								// albedo 
								if (m_isAbedoButtonHovered) {
									m_materialRegistry.LoadTextureForActiveMaterial(graphics::ALBEDO, firstPath);
								}
								// roughness
								if (m_isRoughnessButtonHovered) {
									m_materialRegistry.LoadTextureForActiveMaterial(graphics::ROUGHNESS, firstPath);
								}
								// normal
								if (m_isNormalButtonHovered) {
									m_materialRegistry.LoadTextureForActiveMaterial(graphics::NORMAL, firstPath);
								}
								// metallic
								if (m_isMetallicButtonHovered) {
									m_materialRegistry.LoadTextureForActiveMaterial(graphics::METALLIC, firstPath);
								}
								// ao
								if (m_isAmbientOcclusionButtonHovered) {
									m_materialRegistry.LoadTextureForActiveMaterial(graphics::AMBIENTOCCLUSION, firstPath);
								}
								// emission
								if (m_isEmissionButtonHovered) {
									m_materialRegistry.LoadTextureForActiveMaterial(graphics::EMISSION, firstPath);
								}

								m_readyToDragMaterial = false;


							}
						}
					}
				}
			}
		}

	}

} // !mnemosy::gui
