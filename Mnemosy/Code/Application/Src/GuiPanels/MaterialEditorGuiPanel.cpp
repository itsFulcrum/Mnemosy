#include "Include/GuiPanels/MaterialEditorGuiPanel.h"

#include "Include/Application.h"
#include "Include/ApplicationConfig.h"
#include "Include/MnemosyEngine.h"

#include "Include/Core/FileDirectories.h"
#include "Include/Core/Log.h"
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#include "Include/Core/Utils/DropHandler_Windows.h"
#include "Include/Core/Clock.h"

#include "Include/Systems/Input/InputSystem.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Systems/ExportManager.h"
#include "Include/Systems/FolderTreeNode.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/TextureDefinitions.h"
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
		panelType = MNSY_GUI_PANEL_MATERIAL_EDITOR;

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

		ImGui::Begin(panelName, &showPanel);

		// variables used across entire method
		MnemosyEngine& engineInstance = MnemosyEngine::GetInstance();
		graphics::Material& activeMat = engineInstance.GetScene().GetActiveMaterial();


		std::string displayText = "Material: " + activeMat.Name;
		ImGui::SeparatorText(displayText.c_str());

		bool materialSelected = m_materialRegistry.UserMaterialBound();

		// return early if no material is selcted yet
		if (!materialSelected) {
		
			ImGui::Spacing();

			ImGui::Text("No material selected yet");
			ImGui::End();

			m_currentActiveMaterialID = -1;

			return;
		}

		CheckToSaveMaterial(engineInstance.GetClock().GetDeltaSeconds());

		m_isPanelHovered = ImGui::IsWindowHovered();

		ImVec2 buttonSize = ImVec2(120, 0);

		const char* normalMapFormats[] = { "OpenGl", "DirectX" }; // they need to be ordered the same as in material NormalMapFormat Enum
		const char* exportFormats[] = { "tiff","png"}; // they need to be ordered the same as in ExportManager ExportImageFormats
		
		fs::path libDir = engineInstance.GetFileDirectories().GetLibraryDirectoryPath();
		systems::ExportManager& exportManager = engineInstance.GetExportManager();


		// Check if the active material changed			
		int nowActiveMaterialID = m_materialRegistry.GetActiveMaterialID();

		if (m_currentActiveMaterialID != nowActiveMaterialID) {
			// Active material Changed
			m_currentActiveMaterialID = nowActiveMaterialID;
		}
		

		// Set Drop manager to display cursor if files can be dropped or not
		if ( m_isAbedoButtonHovered 
			|| m_isNormalButtonHovered
			|| m_isRoughnessButtonHovered 
			|| m_isMetallicButtonHovered
			|| m_isAmbientOcclusionButtonHovered
			|| m_isEmissionButtonHovered 
			|| m_isHeightButtonHovered
			|| m_isOpacityButtonHovered ) 
		{
			engineInstance.GetDropHandler().SetDropTargetActive(true);
		}
		else {
			engineInstance.GetDropHandler().SetDropTargetActive(false);
		}



		{

			
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
			

			// === Drag Material to other applications
			bool btn = ImGui::Button("Drag Me", ImVec2(120.0f, 45.0f));

			if (ImGui::IsItemClicked()) {

				std::vector<std::string> filepaths = m_materialRegistry.GetFilepathsOfActiveMat(activeMat);

				if (!filepaths.empty()) {

					engineInstance.GetDropHandler().BeginDrag(filepaths);

					filepaths.clear();
				}
				else {

					MNEMOSY_WARN("No texture files to drag in this material");
				}
			}

			ImGui::Spacing();
			ImGui::Spacing();

			// Export Settings
			{
				if (ImGui::TreeNode("Export Settings")) {

					int exportImageFormat_current = exportManager.GetExportImageFormatInt();
					ImGui::Combo("Image Format ##Export", &exportImageFormat_current, exportFormats, IM_ARRAYSIZE(exportFormats));
					ImGui::SetItemTooltip("Specify the image format used for exporting");

					// if format changed
					if (exportManager.GetExportImageFormatInt() != exportImageFormat_current) {
						exportManager.SetExportImageFormatInt(exportImageFormat_current);
					}


					// Normal Map Format
					int exportNormalFormat_current = exportManager.GetNormalMapExportFormatInt();
					ImGui::Combo("Normal Map Format ##Export", &exportNormalFormat_current, normalMapFormats, IM_ARRAYSIZE(normalMapFormats));
					ImGui::SetItemTooltip("Specify the normal map format to export normal maps with");

					// if format changed
					if (exportManager.GetNormalMapExportFormatInt() != exportNormalFormat_current) {
						exportManager.SetNormalMapExportFormatInt(exportNormalFormat_current);
					}

					bool exportRoughAsSmooth = exportManager.GetExportRoughnessAsSmoothness();
					ImGui::Checkbox("Roughness as Smoothness", &exportRoughAsSmooth);


					if (exportRoughAsSmooth != exportManager.GetExportRoughnessAsSmoothness()) {
						exportManager.SetExportRoughnessAsSmoothness(exportRoughAsSmooth);
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


		}

		ImGui::Spacing();
		ImGui::Spacing();
		const char* readImageFormats = readable_textureFormats_DialogFilter;


		ImVec2 buttonSizeLoad = ImVec2(120, 0);
		ImVec2 buttonSizeDelete = ImVec2(80, 0);

		// Albedo Settings
		{	
			ImGui::SeparatorText("Albedo ");
			// Load Texture
			if (ImGui::Button("Load Texture...##Albedo",buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_ALBEDO, filepath);
					SaveMaterial();
				}
			}
			m_isAbedoButtonHovered = ImGui::IsItemHovered();


			bool textureAssigned = activeMat.isAlbedoAssigned();


			// we cant merge this with the below if statement because the button can change the state of the texture assigned bool
			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Albedo", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_ALBEDO);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			if (textureAssigned) {

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetAlbedoTexture().GetWidth()) + "x" + std::to_string(activeMat.GetAlbedoTexture().GetHeight());
				ImGui::Text(resolution.c_str());

#ifdef mnemosy_gui_showDebugInfo
				// DEBUG
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::ALBEDO));
				ImGui::Text(TextureID.c_str());				
#endif // mnemosy_gui_showDebugInfo
			}

			// Disabled widgets if texture is assigned 
			if(textureAssigned)
				ImGui::BeginDisabled();
			{
			
			
				if (ImGui::ColorEdit3("Albedo Color", (float*)&activeMat.Albedo)) {
					m_valuesChanged = true;
				}

			}
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

					activeMat.IsSmoothnessTexture = false;
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_ROUGHNESS, filepath);
					SaveMaterial();
				}
			}
			m_isRoughnessButtonHovered = ImGui::IsItemHovered();


			// Remove Texture
			if (textureAssigned) {

				ImGui::SameLine();
				if (ImGui::Button("Remove ##Roughness", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_ROUGHNESS);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			// Texture Info text
			if (textureAssigned) {
				
				std::string resolution = "Resolution: " + std::to_string(activeMat.GetRoughnessTexture().GetWidth()) + "x" + std::to_string(activeMat.GetRoughnessTexture().GetHeight());
				ImGui::Text(resolution.c_str());

#ifdef mnemosy_gui_showDebugInfo
				// DEBUG
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::ROUGHNESS));
				ImGui::Text(TextureID.c_str());				
#endif // mnemosy_gui_showDebugInfo


			}
			
			// Disabled widgets if texture is assigned 
			if (textureAssigned)
				ImGui::BeginDisabled();
			{
				if (ImGui::SliderFloat("Roughness", &activeMat.Roughness, 0.0f, 1.0f, "%.4f")) {
					m_valuesChanged = true;
				}

			}
			if (textureAssigned)
				ImGui::EndDisabled();



			// Disabled Widgets if texture is not assiggned
			if (!textureAssigned)
				ImGui::BeginDisabled();
			{
				bool isSmoothness = activeMat.IsSmoothnessTexture;

				ImGui::Checkbox("Is Smoothness", &isSmoothness);

				if (isSmoothness != activeMat.IsSmoothnessTexture) {

					MNEMOSY_TRACE("Converting Roughness texture");
					// roughness changed update
					activeMat.IsSmoothnessTexture = isSmoothness;


					//MNEMOSY_TRACE("Changed isSmoothneess to: {}", activeMat.IsSmoothnessTexture);

					fs::path materialFolderPath = libDir / m_materialRegistry.m_folderNodeOfActiveMaterial->pathFromRoot / fs::path(activeMat.Name);

					
					fs::path roughnessPath = materialFolderPath / fs::path(std::string(activeMat.Name + texture_fileSuffix_roughness));


					MnemosyEngine::GetInstance().GetTextureGenerationManager().InvertRoughness(activeMat, roughnessPath.generic_string().c_str(), true);



					MNEMOSY_TRACE("Converted");



					// TODO:
					// Load newly created texture as new roughness texture 
					activeMat.GetRoughnessTexture().GenerateFromFile(roughnessPath.generic_string().c_str(), true, true);

					MNEMOSY_TRACE("Loaded New Roughness texture");




					// save is Smoothness to data file
					activeMat.IsSmoothnessTexture = isSmoothness;
					
					SaveMaterial();
					MNEMOSY_TRACE("SavedMaterialFile");

				}

			}
			if (!textureAssigned)
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
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_NORMAL, filepath);
					SaveMaterial();
				}
			}
			m_isNormalButtonHovered = ImGui::IsItemHovered();

			// Remove Texture
			if (textureAssigned) {
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Normal", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_NORMAL);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			// Texture Info Text
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

				if (ImGui::DragFloat("Normal Strength", &activeMat.NormalStrength, 0.01f, 0.0f, 100.0f, "%.4f")) {
					m_valuesChanged = true;
				}
				
				// NORMAL MAP FORMAT
				
				int format_current = activeMat.GetNormalFormatAsInt();

				ImGui::Combo("Normal Map Format", &format_current, normalMapFormats, IM_ARRAYSIZE(normalMapFormats));
				ImGui::SetItemTooltip("Specify the normal map format of the source texture");

				// if format changed
				if (activeMat.GetNormalFormatAsInt() != format_current) {


					fs::path materialFolderPath = libDir / fs::path(m_materialRegistry.m_folderNodeOfActiveMaterial->pathFromRoot) / fs::path(activeMat.Name);
					fs::path normalMapPath = materialFolderPath / fs::path(std::string(activeMat.Name + texture_fileSuffix_normal));
										
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

					activeMat.GetNormalTexture().GenerateFromFile(normalMapPath.generic_string().c_str(),true,true);

					SaveMaterial();

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
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_METALLIC, filepath);
					SaveMaterial();
				}
			}
			m_isMetallicButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Metallic", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_METALLIC);
					textureAssigned = false;
					SaveMaterial();
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

				if (ImGui::SliderFloat("Metallic", &activeMat.Metallic, 0.0f, 1.0f, "%.4f")) {
					m_valuesChanged = true;
				}
			}
			if (textureAssigned)
				ImGui::EndDisabled();

		} // End Metallic Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Ambient Occlusion Settings
		{ 
			ImGui::SeparatorText("Ambient Occlusion ");
			bool textureAssigned = activeMat.isAoAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##AO", buttonSizeLoad)) {
				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);
				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_AMBIENTOCCLUSION, filepath);
					SaveMaterial();
				}
			}
			m_isAmbientOcclusionButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##AO",buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_AMBIENTOCCLUSION);
					textureAssigned = false;
					SaveMaterial();
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
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_EMISSION, filepath);
					SaveMaterial();

				}
			}
			m_isEmissionButtonHovered = ImGui::IsItemHovered();


			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Emissive", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_EMISSION);
					textureAssigned = false;
					SaveMaterial();
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




			if (!textureAssigned) 
				ImGui::BeginDisabled();
			
			{
			
				bool useEmitAsMask = activeMat.UseEmissiveAsMask;
				ImGui::Checkbox("Use Emissive As Mask", &useEmitAsMask);

				if (useEmitAsMask != activeMat.UseEmissiveAsMask) {
					activeMat.UseEmissiveAsMask = useEmitAsMask;
					m_valuesChanged = true;
			
				}

			}
			if (!textureAssigned)
				ImGui::EndDisabled();



			// Disabled widgets if texture is assigned 
			if (textureAssigned && !activeMat.UseEmissiveAsMask)
				ImGui::BeginDisabled();
			{
				

				if (ImGui::ColorEdit3("Emission", (float*)&activeMat.Emission)) {
					m_valuesChanged = true;
				}

			}
			if (textureAssigned && !activeMat.UseEmissiveAsMask)
				ImGui::EndDisabled();


			if (ImGui::DragFloat("Emission Strength", &activeMat.EmissionStrength, 0.01f, 0.0f, 10000.0f, "%.4f")) {
				m_valuesChanged = true;
			}

		} // End Emission Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Height Settings
		{
			ImGui::SeparatorText("Height ");
			bool textureAssigned = activeMat.isHeightAssigned();


			// Load Texture
			if (ImGui::Button("Load Texture...##Height", buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);

				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_HEIGHT, filepath);
					SaveMaterial();
				}
			}

			m_isHeightButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Height", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_HEIGHT);
					textureAssigned = false;
					SaveMaterial();
				}

			}

			if (textureAssigned) {

				// render info text
				std::string resolution = "Resolution: " + std::to_string(activeMat.GetHeightTexture().GetWidth()) + "x" + std::to_string(activeMat.GetHeightTexture().GetHeight());
				ImGui::Text(resolution.c_str());


#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::MNSY_TEXTURE_HEIGHT));
				ImGui::Text(TextureID.c_str());
#endif // mnemosy_gui_showDebugInfo

			}





			// Disabled widgets if texture is not assigned 
			if (!textureAssigned)
				ImGui::BeginDisabled();
			{


				if (ImGui::SliderFloat("Height Depth", &activeMat.HeightDepth, 0.0f, 10.0f, "%.4f")) {
					m_valuesChanged = true;
				}

			}
			if (!textureAssigned)
				ImGui::EndDisabled();




		}
		ImGui::Spacing();
		ImGui::Spacing();

		// Opacity Settings
		{

			ImGui::SeparatorText("Opacity ");

			bool textureAssigned = activeMat.isOpacityAssigned();


			// Load Texture
			if (ImGui::Button("Load Texture...##Opacity", buttonSizeLoad)) {

				std::string filepath = mnemosy::core::FileDialogs::OpenFile(readImageFormats);

				if (!filepath.empty()) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_OPACITY, filepath);
					SaveMaterial();
				}
			}

			if (!textureAssigned && activeMat.isAlbedoAssigned()) {


				if (ImGui::Button("Generate from albedo alpha",ImVec2(230, 0))) {


					m_materialRegistry.GenereateOpacityFromAlbedoAlpha(activeMat);



				}

			}


			m_isOpacityButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Remove ##Opacity", buttonSizeDelete)) {
					m_materialRegistry.DeleteTextureOfActiveMaterial(graphics::MNSY_TEXTURE_OPACITY);
					textureAssigned = false;
					SaveMaterial();
				}

			}

			if (textureAssigned) {
				// render info text
				std::string resolution = "Resolution: " + std::to_string(activeMat.GetOpacityTexture().GetWidth()) + "x" + std::to_string(activeMat.GetOpacityTexture().GetHeight());
				ImGui::Text(resolution.c_str());


#ifdef mnemosy_gui_showDebugInfo
				std::string TextureID = "Debug: GL TexID: " + std::to_string(activeMat.DebugGetTextureID(graphics::MNSY_TEXTURE_OPACITY));
				ImGui::Text(TextureID.c_str());
#endif // mnemosy_gui_showDebugInfo



			}


			// settings

			// Disabled widgets if texture is assigned 
			if (!textureAssigned)
				ImGui::BeginDisabled();
			{


				if (ImGui::SliderFloat("Opacity Threshold", &activeMat.OpacityTreshhold, 0.0f, 1.0f, "%.4f")) {
					m_valuesChanged = true;
				}

			}
			if (!textureAssigned)
				ImGui::EndDisabled();



		}

		ImGui::Spacing();
		ImGui::Spacing();



		// General Settings
		{
			ImGui::SeparatorText("General ");

			float uvTiling[2] = { activeMat.UVTiling.x,activeMat.UVTiling.y };
			if (ImGui::DragFloat2("UV Tiling", (float*)uvTiling, 0.1f, 0.0, 1000, "%0.4f")) {
				activeMat.UVTiling.x = uvTiling[0];
				activeMat.UVTiling.y = uvTiling[1];

				m_valuesChanged = true;
			}
		} // End General Settings
		
		ImGui::Spacing();
		ImGui::Spacing();


		if (m_materialRegistry.UserMaterialBound()) {

			if (ImGui::Button("Save Material", buttonSize)) {

				SaveMaterial();
			}
		}


		ImGui::End();

	} // End Draw()

	// Callback when files are droped into mnemosy
	void MaterialEditorGuiPanel::OnFileDropInput(int count, std::vector<std::string>& dropedFilePaths) {

		if (m_isPanelHovered) {


			if (count == 0)
				return;

			if (dropedFilePaths.empty()) // just making sure
				return;



			// we kinda only want to handle the first one.
			std::string firstPath = dropedFilePaths[0];
			fs::directory_entry firstFile = fs::directory_entry(firstPath);


			if (!firstFile.exists()) 
				return;

			if (!firstFile.is_regular_file())
				return;

			std::string extention = firstFile.path().extension().generic_string();
			if (extention == ".png" || extention == ".tif" || extention == ".tiff" || extention == ".jpg" || extention == ".jpeg") {
				//MNEMOSY_DEBUG("MaterialEditorGuiPanel::OnFileDropInput: FirstFile is Valid File. Filetype: {}",extention);
				// Valid texture file


				// albedo 
				if (m_isAbedoButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_ALBEDO, firstPath);
				}
				// roughness
				else if (m_isRoughnessButtonHovered) {
					
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_ROUGHNESS, firstPath);
				}
				// normal
				else if (m_isNormalButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_NORMAL, firstPath);
				}
				// metallic
				else if (m_isMetallicButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_METALLIC, firstPath);
				}
				// ao
				else if (m_isAmbientOcclusionButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_AMBIENTOCCLUSION, firstPath);
				}
				// emission
				else if (m_isEmissionButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_EMISSION, firstPath);
				}
				// height
				else if (m_isHeightButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_HEIGHT, firstPath);
				}
				// opacity
				else if (m_isOpacityButtonHovered) {
					m_materialRegistry.LoadTextureForActiveMaterial(graphics::MNSY_TEXTURE_OPACITY, firstPath);
				}


				SaveMaterial();

			}
			
			
				
			
		}

	}

	void MaterialEditorGuiPanel::CheckToSaveMaterial(float deltaSeconds) {

		// if valuesChanged is set run a timer until we save the material, this will also generate a new thumbnail so we don't want to do it imidiatly to save some computation cost.

		if (m_valuesChanged) {

			m_TimeToSaveMaterialDelta += deltaSeconds;

			if (m_TimeToSaveMaterialDelta >= m_TimeToSaveMaterial) {
				m_TimeToSaveMaterialDelta = 0.0f;
				SaveMaterial();
			}
		}
	}

	void MaterialEditorGuiPanel::SaveMaterial()
	{
		m_materialRegistry.SaveActiveMaterialToFile();
		m_valuesChanged = false;
	}

} // !mnemosy::gui
