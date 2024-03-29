#include "Include/GuiPanels/MaterialEditorGuiPanel.h"

#include "Include/Application.h"
#include "Include/ApplicationConfig.h"
#include "Include/MnemosyEngine.h"

#include "Include/Systems/MaterialLibraryRegistry.h"

#include "Include/Core/Utils/PlatfromUtils_Windows.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Texture.h"

#include "External/ImGui/imgui.h"

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

			if (ImGui::Button("Save Material")) {
					
				m_materialRegistry.SaveActiveMaterialToFile();
			}

		}

		ImGui::Spacing();
		ImGui::Spacing();
		const char* fileFormats = "TIF (*.tif)\0*.tif\0 HDR (*.hdr)\0*.hdr\0 PNG (*.png)\0*.png\0 JPG (*.jpg)\0*.jpg\0 TIFF (*.tiff)\0*.tiff\0";
		ImVec2 buttonSizeLoad = ImVec2(120, 0);
		ImVec2 buttonSizeDelete = ImVec2(80, 0);

		// Albedo Settings
		{	
			ImGui::SeparatorText("Albedo ");
			// Load Texture
			if (ImGui::Button("Load Texture...##Albedo",buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::ALBEDO, filepath);
				}
			}

			bool textureAssigned = activeMat.isAlbedoAssigned();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Albedo", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::ALBEDO);
				}

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				ImGui::SameLine();
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

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::ROUGHNESS, filepath);
				}
			}

			if (textureAssigned) {

				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Roughness", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::ROUGHNESS);
				}

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				ImGui::SameLine();
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

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::NORMAL, filepath);
				}
			}

			if (textureAssigned) {

				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Normal", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::NORMAL);
				}

				// DEBUG INFO
#ifdef mnemosy_gui_showDebugInfo
					ImGui::SameLine();
					std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::NORMAL));
					ImGui::Text(TextureID.c_str());
#endif // mnemosy_gui_showDebugInfo
			}

			// Disabled widgets if texture is NOT assigned 
			if (!textureAssigned)
				ImGui::BeginDisabled();
			{
				ImGui::DragFloat("Normal Strength", &activeMat.NormalStrength,0.01f, 0.0f, 100.0f, "%.4f");
			
				// TODO: 
				// Normal map format Combo Setting

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
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::METALLIC, filepath);
				}
			}

			if (textureAssigned) {

				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Metallic", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::METALLIC);
				}

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				ImGui::SameLine();
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
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::AMBIENTOCCLUSION, filepath);
				}
			}

			if (textureAssigned) {

				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##AO",buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::AMBIENTOCCLUSION);
				}

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				ImGui::SameLine();
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
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(fileFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::EMISSION, filepath);
				}
			}

			if (textureAssigned) {

				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Emissive", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::EMISSION);
				}

				// DEBUG
#ifdef mnemosy_gui_showDebugInfo
				ImGui::SameLine();
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
		

		ImGui::End();

	} // End Draw()

} // !mnemosy::gui
