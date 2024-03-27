#include "Include/GuiPanels/MaterialEditorGuiPanel.h"

#include "Include/Application.h"
#include "Include/MnemosyEngine.h"
//#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Texture.h"
#include "External/ImGui/imgui.h"

#include "Include/MnemosyConfig.h"
#ifdef MNEMOSY_PLATFORM_WINDOWS
	#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

#include <string>
#include <glm/glm.hpp>
#include <filesystem>


namespace mnemosy::gui
{	
	MaterialEditorGuiPanel::MaterialEditorGuiPanel()
		: m_materialRegistry{ ENGINE_INSTANCE().GetMaterialLibraryRegistry() }
	{
		panelName = "Material Editor";
	}

	void MaterialEditorGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		graphics::Material& activeMat = ENGINE_INSTANCE().GetScene().GetActiveMaterial();

		ImGui::Begin(panelName.c_str(), &showPanel);

		{
			std::string displayText = "Material: " + activeMat.Name;
			ImGui::SeparatorText(displayText.c_str());

			if (m_materialRegistry.UserMaterialBound()) {

				{ // only for debug purposes
					std::filesystem::path matDataFile = m_materialRegistry.GetActiveMaterialDataFilePath();
					std::string matDataFileString = 
						"Active Material: " + activeMat.Name + 
						"\nActive ID: " + std::to_string(m_materialRegistry.GetActiveMaterialID()) + 
						//"\n Folder: " + m_materialRegistry.m_folderNodeOfActiveMaterial.name +
						"\nDataFilePath: " + matDataFile.generic_string();
					ImGui::Text(matDataFileString.c_str());
				}
				

				if (ImGui::Button("Save Material")) {
					
					m_materialRegistry.SaveActiveMaterialToFile();
				}

			}

			ImGui::Spacing();

			ImGui::ColorEdit3("Albedo", (float*)&activeMat.Albedo);
			ImGui::SliderFloat("Roughness", &activeMat.Roughness, 0.0f, 1.0f, "%.4f");
			ImGui::SliderFloat("Metallic", &activeMat.Metallic, 0.0f, 1.0f, "%.4f");
			ImGui::ColorEdit3("Emission", (float*)&activeMat.Emission);
			ImGui::DragFloat("Emission Strength", &activeMat.EmissionStrength,0.01f, 0.0f, 10000.0f, "%.4f");
			ImGui::DragFloat("Normal Strength", &activeMat.NormalStrength,0.01f, 0.0f, 100.0f, "%.4f");

			float uvTiling[2] = { 1.0f,1.0f };
			uvTiling[0] = activeMat.UVTiling.x;
			uvTiling[1] = activeMat.UVTiling.y;
			ImGui::DragFloat2("UV Tiling", (float*)uvTiling, 0.1f, 0.0, 1000, "%0.4f");
			activeMat.UVTiling.x = uvTiling[0];
			activeMat.UVTiling.y = uvTiling[1];


#ifdef MNEMOSY_PLATFORM_WINDOWS


			const char* fileFormats = "TIF (*.tif)\0*.tif\0 HDR (*.hdr)\0*.hdr\0 PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0 TIFF (*.tiff)\0*.tiff\0";

			if (ImGui::Button("Load Albedo...")) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::ALBEDO, filepath);
				}
			}
			if (activeMat.isAlbedoAssigned()) {
				ImGui::SameLine();
				if(ImGui::Button("Delete Albedo")) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::ALBEDO);
				}
				ImGui::SameLine();
				// DEBUG
				std::string TextureID = "ID " + std::to_string( activeMat.DebugGetTextureID(graphics::ALBEDO));
				ImGui::Text(TextureID.c_str());
			}

			if (ImGui::Button("Load Normal...")) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::NORMAL, filepath);
				}
			}
			if (activeMat.isNormalAssigned()) {
				ImGui::SameLine();
				if (ImGui::Button("Delete Normal")) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::NORMAL);
				}
				// DEBUG
				std::string TextureID = "ID " + std::to_string(activeMat.DebugGetTextureID(graphics::NORMAL));
				ImGui::Text(TextureID.c_str());
			}
			// Roughness
			if (ImGui::Button("Load Roughness...")) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::ROUGHNESS, filepath);
				}
			}
			if (activeMat.isRoughnessAssigned()) {
				ImGui::SameLine();
				if (ImGui::Button("Delete Roughness")) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::ROUGHNESS);
				}

				// DEBUG
				std::string TextureID = "ID " + std::to_string(activeMat.DebugGetTextureID(graphics::ROUGHNESS));
				ImGui::Text(TextureID.c_str());
			}

			// Metallic
			if (ImGui::Button("Load Metallic...")) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::METALLIC, filepath);
				}
			}
			if (activeMat.isMetallicAssigned()) {
				ImGui::SameLine();
				if (ImGui::Button("Delete Metallic")) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::METALLIC);
				}
			}

			// Ambient Occlusion
			if (ImGui::Button("Load Ambient Occlusion...")) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::AMBIENTOCCLUSION, filepath);
				}
			}
			if (activeMat.isAoAssigned()) {
				ImGui::SameLine();
				if (ImGui::Button("Delete Ambient Occlusion")) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::AMBIENTOCCLUSION);
				}
			}
			// Emission 
			if (ImGui::Button("Load Emissive...")) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::EMISSION, filepath);
				}
			}
			if (activeMat.isEmissiveAssigned()) {
				ImGui::SameLine();
				if (ImGui::Button("Delete Emissive")) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::EMISSION);
				}
			}

#endif



			
		}


		ImGui::End();

	}

}
