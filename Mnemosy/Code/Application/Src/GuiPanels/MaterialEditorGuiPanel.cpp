#include "Include/GuiPanels/MaterialEditorGuiPanel.h"

#include "Include/ApplicationConfig.h"
#include "Include/MnemosyEngine.h"

#include "Include/Core/Clock.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#include "Include/Core/Utils/DropHandler_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

#include "Include/Gui/UserInterface.h"
#include "Include/Core/Utils/StringUtils.h"

#include "Include/Systems/Input/InputSystem.h"
#include "Include/Systems/MaterialLibraryRegistry.h"
#include "Include/Systems/TextureGenerationManager.h"
#include "Include/Systems/ExportManager.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Systems/SkyboxAssetRegistry.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Graphics/RenderMesh.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/TextureDefinitions.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/Utils/Picture.h"

#include "Include/GuiPanels/GuiPanelsCommon.h"

#include <glm/glm.hpp>

namespace mnemosy::gui
{	
	MaterialEditorGuiPanel::MaterialEditorGuiPanel()
		: m_engineInstance{MnemosyEngine::GetInstance()}
		, m_materialRegistry{ MnemosyEngine::GetInstance().GetMaterialLibraryRegistry()}
		, m_exportManager{MnemosyEngine::GetInstance().GetExportManager()}
	{
		panelName = "Material Editor";
		panelType = MNSY_GUI_PANEL_MATERIAL_EDITOR;

		m_onFileDropInput_callback_id = MnemosyEngine::GetInstance().GetInputSystem().REGISTER_DROP_INPUT(&MaterialEditorGuiPanel::OnFileDropInput);
	
		m_textureTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
		
		m_pbrMat_exportTextureBools = std::vector<bool>((int)graphics::PBRTextureType::MNSY_TEXTURE_COUNT,true);
	}

	MaterialEditorGuiPanel::~MaterialEditorGuiPanel() {

		MnemosyEngine::GetInstance().GetInputSystem().UnregisterDropInput(m_onFileDropInput_callback_id);
		m_pbrMat_exportTextureBools.clear();
	}

	void MaterialEditorGuiPanel::Draw() {
		if (!showPanel)
			return;

		ImGui::Begin(panelName, &showPanel);

		
		systems::LibEntry* libEntry = m_materialRegistry.ActiveLibEntry_Get();

		// return early if no entry is selcted yet
		if (libEntry == nullptr) {
		
			ImGui::Spacing();

			ImGui::Text("No material selected yet");
			ImGui::End();

			m_lastActiveLibEntry_ID = -1;

			return;
		}

		std::string displayText = "Entry: " + libEntry->name;
		ImGui::SeparatorText(displayText.c_str());

		CheckToSaveMaterial(m_engineInstance.GetClock().GetDeltaSeconds());

		m_isPanelHovered = ImGui::IsWindowHovered();

		

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
			

		// === Drag Lib Entry Textures to other applications
		ImGui::Button("Drag Me", m_buttonDrag);

		if (ImGui::IsItemClicked()) {

			std::vector<std::string> filepaths = m_materialRegistry.ActiveLibEntry_GetTexturePaths();

			if (!filepaths.empty()) {

				m_engineInstance.GetDropHandler().BeginDrag(filepaths);

				filepaths.clear();
			}
			else {

				MNEMOSY_WARN("No texture files to drag in this material entry");
			}
		}

		ImGui::SetItemTooltip("Drag & Drop texture files into another program");

		ImGui::Spacing();
		ImGui::Spacing();

		DrawExportSettings(libEntry);

		if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

			graphics::PbrMaterial& pbrMat = m_engineInstance.GetScene().GetPbrMaterial();

			DrawPbrMaterial(libEntry,pbrMat);
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT)
		{
			graphics::UnlitMaterial* unlit = m_engineInstance.GetScene().GetUnlitMaterial();

			DrawEntryUnlitMat(libEntry, unlit);
		}
		else if (libEntry->type == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX)
		{

			graphics::Skybox* skybox = &m_engineInstance.GetScene().GetSkybox();

			DrawEntrySkybox(libEntry, skybox);
		}		

		ImGui::End();

	} // End Draw()

	void MaterialEditorGuiPanel::DrawExportSettings(systems::LibEntry* activeLibEntry)
	{
		// Check if the active material changed			
		int currentActiveMaterialID = activeLibEntry->runtime_ID;

		systems::LibEntryType entryType = activeLibEntry->type;

		// check if entry selection changed from last frame
		if (m_lastActiveLibEntry_ID != currentActiveMaterialID) {
			
			// selected entry Changed
			m_lastActiveLibEntry_ID = currentActiveMaterialID;


			// reevaluate entry type specific export settings
			if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

				m_pbrMat_exportChannelPackedTexture = true;

				for (int i = 0; i < m_pbrMat_exportTextureBools.size(); i++) {
					m_pbrMat_exportTextureBools[i] = true;
				}
			}

		}



		if (ImGui::TreeNode("Export ##Settings")) {

			int exportImageFormat_current = (int)m_exportManager.GetExportImageFormat();

			ImGui::Combo("Image Format ##Export", &exportImageFormat_current, graphics::TexDefinitions::ImageFileFormats_string, IM_ARRAYSIZE(graphics::TexDefinitions::ImageFileFormats_string));

			ImGui::SetItemTooltip("Specify the image format used for exporting");

			// if export img format changed
			if ((int)m_exportManager.GetExportImageFormat() != exportImageFormat_current) {

				m_exportManager.SetExportImageFormat((graphics::ImageFileFormat)exportImageFormat_current);
			}


			graphics::Scene& scene = MnemosyEngine::GetInstance().GetScene();


			// Pbr Material specific settings
			if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

				// Normal Map Format
				int exportNormalFormat_current = (int)m_exportManager.GetNormalMapExportFormat();
				ImGui::Combo("Normal Map Format ##Export", &exportNormalFormat_current, graphics::TexDefinitions::NormalMapFormats_string, IM_ARRAYSIZE(graphics::TexDefinitions::NormalMapFormats_string));
				ImGui::SetItemTooltip("Specify the format for exporting Normal Maps");

				// if normal format changed
				if ((int)m_exportManager.GetNormalMapExportFormat() != exportNormalFormat_current) {
					m_exportManager.SetNormalMapExportFormat((graphics::NormalMapFormat)exportNormalFormat_current);
				}

				bool exportRoughAsSmooth = m_exportManager.GetExportRoughnessAsSmoothness();
				ImGui::Checkbox("Roughness as Smoothness", &exportRoughAsSmooth);
				ImGui::SetItemTooltip("Export Roughness as Smoothness map");

				if (exportRoughAsSmooth != m_exportManager.GetExportRoughnessAsSmoothness()) {
					m_exportManager.SetExportRoughnessAsSmoothness(exportRoughAsSmooth);
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				graphics::PbrMaterial& pbrMat = scene.GetPbrMaterial();

				// make a export checkbox for each pbr texture type
				for (int i = 0; i < (int)graphics::PBRTextureType::MNSY_TEXTURE_COUNT; i++) {


					graphics::PBRTextureType texType = (graphics::PBRTextureType)i;

					std::string textureTypeName = graphics::TexUtil::get_string_from_PBRTextureType(texType);
					std::string label = "Export " + textureTypeName;

					bool isAssigned = pbrMat.IsTextureTypeAssigned(texType);
					if (!isAssigned) {

						if (m_pbrMat_exportTextureBools[i] == true) {
							m_pbrMat_exportTextureBools[i] = false;
						}

						ImGui::BeginDisabled();
					}

					bool exportTexture = m_pbrMat_exportTextureBools[i];


					ImGui::Checkbox(label.c_str(), &exportTexture);

					if (exportTexture != m_pbrMat_exportTextureBools[i]) {

						m_pbrMat_exportTextureBools[i] = exportTexture;
					}


					if (!isAssigned)
						ImGui::EndDisabled();

				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();


				if (!pbrMat.HasPackedTextures) {

					if (m_pbrMat_exportChannelPackedTexture == true) {
						m_pbrMat_exportChannelPackedTexture = false;
					}

					ImGui::BeginDisabled();
				}

				ImGui::Checkbox("Export Channel Packed Textures", &m_pbrMat_exportChannelPackedTexture);

				if (!pbrMat.HasPackedTextures) {

					ImGui::EndDisabled();
				}

			}

			// no specific settings yet
			//else if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {
			//
			//	// no specific settings yet
			//}

			//else if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

			//	// no specific settings yet
			//}


			// === Export Button

			if (ImGui::Button("Export To...", m_buttonSize)) {

				std::filesystem::path exportPath = mnemosy::core::FileDialogs::SelectFolder("");
				if (!exportPath.empty()) {


					if (fs::is_directory(exportPath)) {


						m_materialRegistry.ActiveLibEntry_SaveToFile();
					
						//fs::path exportPath = fs::path(exportFolder);
					
						if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {
						
							m_exportManager.PbrMat_ExportTextures(exportPath, activeLibEntry , scene.GetPbrMaterial() , m_pbrMat_exportTextureBools, m_pbrMat_exportChannelPackedTexture);					
						}
						else if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {


							graphics::UnlitMaterial* unlitMat = scene.GetUnlitMaterial();

							MNEMOSY_ASSERT(unlitMat != nullptr, "This should not happen!");

							m_exportManager.UnlitMat_ExportTextures(exportPath,activeLibEntry, *unlitMat);
						}
						else if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

							m_exportManager.SkyboxMat_ExportTextures(exportPath,activeLibEntry, scene.GetSkybox());
						}
					}
				}
			}


			if (entryType == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
		
				if (ImGui::Button("Add to Quick Selection", ImVec2(180, 0))) {

					m_engineInstance.GetSkyboxAssetRegistry().AddLibEntryToPreviewSkyboxes(activeLibEntry);
					m_engineInstance.GetRenderer().SetShaderSkyboxUniforms(scene.userSceneSettings,scene.GetSkybox());
				}
			}



			ImGui::TreePop();
		}

		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();


	}

	void MaterialEditorGuiPanel::DrawPbrMaterial(systems::LibEntry* activeLibEntry, graphics::PbrMaterial& activeMat)
	{

		fs::path libraryDirectory = m_materialRegistry.ActiveLibCollection_GetFolderPath();

		ImGui::Spacing();
		ImGui::Spacing();

		DrawPbrMatTextureSettings(activeLibEntry,activeMat, libraryDirectory);

		DrawPbrMatChannelPackUI(activeLibEntry,activeMat);

	}

	void MaterialEditorGuiPanel::DrawPbrMatTextureSettings(systems::LibEntry* activeLibEntry, graphics::PbrMaterial& activeMat, std::filesystem::path& libDir) {

		ImGui::SeparatorText("Settings");


		ImGui::Spacing();
		ImGui::Spacing();

		// General Settings
		if (ImGui::TreeNodeEx("General Settings", m_textureTreeNodeFlags)) {

			float uvTiling[2] = { activeMat.UVTiling.x,activeMat.UVTiling.y };
			if (ImGui::DragFloat2("UV Tiling", (float*)uvTiling, 0.1f, 0.0, 1000, "%0.4f")) {
				activeMat.UVTiling.x = uvTiling[0];
				activeMat.UVTiling.y = uvTiling[1];

				m_valuesChanged = true;
			}

			ImGui::TreePop();
		} // End General Settings

		ImGui::Spacing();
		ImGui::Spacing();


		// Albedo Settings
		if(ImGui::TreeNodeEx("Albedo ##Settings", m_textureTreeNodeFlags)) 
		{
			// Load Texture
			if (ImGui::Button("Load Texture...##Albedo", m_buttonSizeLoad)) {

				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_ALBEDO, filepath);
					SaveMaterial();
				}
			}
			m_isAbedoButtonHovered = ImGui::IsItemHovered();


			bool textureAssigned = activeMat.isAlbedoAssigned();


			// we cant merge this with the below if statement because the button can change the state of the texture assigned bool
			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Delete ##Albedo", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_ALBEDO);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			if (textureAssigned) {

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetAlbedoTexture().GetWidth()) + "x" + std::to_string(activeMat.GetAlbedoTexture().GetHeight());
				ImGui::Text(resolution.c_str());


			}

			// Disabled widgets if texture is assigned 
			if (textureAssigned)
				ImGui::BeginDisabled();
			{


				if (ImGui::ColorEdit3("Albedo Color", (float*)&activeMat.Albedo)) {
					m_valuesChanged = true;
				}

			}
			if (textureAssigned)
				ImGui::EndDisabled();

			ImGui::TreePop();
		} // End Albedo Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Roughness Settings
		if (ImGui::TreeNodeEx("Roughness ##Settings", m_textureTreeNodeFlags))
		{
			//ImGui::SeparatorText("Roughness ");
			bool textureAssigned = activeMat.isRoughnessAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##Roughness", m_buttonSizeLoad)) {

				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
				if (!filepath.empty()) {

					activeMat.IsSmoothnessTexture = false;
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_ROUGHNESS, filepath);
					SaveMaterial();
				}
			}
			m_isRoughnessButtonHovered = ImGui::IsItemHovered();


			// Remove Texture
			if (textureAssigned) {

				ImGui::SameLine();
				if (ImGui::Button("Delete ##Roughness", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_ROUGHNESS);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			// Texture Info text
			if (textureAssigned) {

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetRoughnessTexture().GetWidth()) + "x" + std::to_string(activeMat.GetRoughnessTexture().GetHeight());
				ImGui::Text(resolution.c_str());
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

				gui::GuiProcedures::SamelineTooltip("Convert imported texture to roughness.\n Use this if you have imported a smoothness/gloss texture.");

				if (isSmoothness != activeMat.IsSmoothnessTexture) {

					MNEMOSY_TRACE("Converting Roughness texture");
					// roughness changed update
					activeMat.IsSmoothnessTexture = isSmoothness;

					systems::LibEntry* entry = m_materialRegistry.ActiveLibEntry_Get();

					fs::path materialFolderPath = m_materialRegistry.LibEntry_GetFolderPath(entry);

					fs::path roughnessPath = materialFolderPath / fs::u8path(entry->name + texture_fileSuffix_roughness);

					MnemosyEngine::GetInstance().GetTextureGenerationManager().InvertRoughness(activeMat, roughnessPath.generic_string().c_str(), true);

					// Load newly created texture as new roughness texture 

					graphics::PictureError picErrorCheck;
					graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(picErrorCheck,roughnessPath.generic_string().c_str(),true,false,true);

					if (!picErrorCheck.wasSuccessfull) {

						MNEMOSY_WARN("Faild to read newly created roughness texture: {}, \nMessage: {}",roughnessPath.generic_string(),picErrorCheck.what);
					}
					else {
						MNEMOSY_TRACE("Loaded New Roughness texture");
						graphics::Texture* roughTex = new graphics::Texture();

						roughTex->GenerateOpenGlTexture(picInfo,true);
						activeMat.assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_ROUGHNESS, roughTex);
						free(picInfo.pixels);
					}



					// its better to save this even if reading the new texture failed because it was aperantly written and if we dont update the meta data it will be out of sync
					activeMat.IsSmoothnessTexture = isSmoothness;

					SaveMaterial();
					MNEMOSY_TRACE("SavedMaterialFile");
				}

			}
			if (!textureAssigned)
				ImGui::EndDisabled();




			ImGui::TreePop();
		} // End Roughness Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Normal Settings
		if (ImGui::TreeNodeEx("Normal ##Settings", m_textureTreeNodeFlags))
		{
			//ImGui::SeparatorText("Normal ");
			bool textureAssigned = activeMat.isNormalAssigned();

			//  Load Texture
			if (ImGui::Button("Load Texture...##Normal", m_buttonSizeLoad)) {

				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_NORMAL, filepath);
					SaveMaterial();
				}
			}
			m_isNormalButtonHovered = ImGui::IsItemHovered();

			// Remove Texture
			if (textureAssigned) {
				ImGui::SameLine();
				if (ImGui::Button("Delete ##Normal", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_NORMAL);
					textureAssigned = false;
					SaveMaterial();
				}
		}

			// Texture Info Text
			if (textureAssigned) {


				std::string resolution = "Resolution: " + std::to_string(activeMat.GetNormalTexture().GetWidth()) + "x" + std::to_string(activeMat.GetNormalTexture().GetHeight());
				ImGui::Text(resolution.c_str());

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

				ImGui::Combo("Normal Map Format", &format_current, graphics::TexDefinitions::NormalMapFormats_string, IM_ARRAYSIZE(graphics::TexDefinitions::NormalMapFormats_string));
				ImGui::SetItemTooltip("Specify the Normal Map format of the source texture");

				// if format changed
				if (activeMat.GetNormalFormatAsInt() != format_current) {

					fs::path materialFolderPath = m_materialRegistry.LibEntry_GetFolderPath(activeLibEntry);
					fs::path normalMapPath = materialFolderPath / fs::u8path(activeLibEntry->name + texture_fileSuffix_normal);

					// Generate inverted normal Texture.
					MnemosyEngine::GetInstance().GetTextureGenerationManager().FlipNormalMap(normalMapPath.generic_string().c_str(), activeMat, true);



					// save normal format to material data file.
					if (format_current == 0) {
						activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_OPENGL);
					}
					else if (format_current == 1) {
						activeMat.SetNormalMapFormat(graphics::MNSY_NORMAL_FORMAT_DIRECTX);
					}
					m_materialRegistry.ActiveLibEntry_SaveToFile();

					// Load newly created texture as new normal map 

					graphics::PictureError picErrorCheck;
					graphics::PictureInfo picInfo = graphics::Picture::ReadPicture(picErrorCheck, normalMapPath.generic_string().c_str(), true, true,true);

					if (!picErrorCheck.wasSuccessfull) {

						MNEMOSY_WARN("Faild to read newly created normal map texture: {}, \nMessage: {}", normalMapPath.generic_string(), picErrorCheck.what);
					}
					else {
						MNEMOSY_TRACE("Loaded New Roughness texture");
						graphics::Texture* normalTex = new graphics::Texture();
						normalTex->GenerateOpenGlTexture(picInfo,true);
						free(picInfo.pixels);
						activeMat.assignTexture(graphics::PBRTextureType::MNSY_TEXTURE_NORMAL, normalTex);
					}

					SaveMaterial();

				}



			}
			if (!textureAssigned)
				ImGui::EndDisabled();


			ImGui::TreePop();
		} // End Normal Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Metallic Settings
		if (ImGui::TreeNodeEx("Metallic ##Settings", m_textureTreeNodeFlags))
		{
			//ImGui::SeparatorText("Metallic ");
			bool textureAssigned = activeMat.isMetallicAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##Metallic", m_buttonSizeLoad)) {
				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_METALLIC, filepath);
					SaveMaterial();
				}
			}
			m_isMetallicButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Delete ##Metallic", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_METALLIC);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			if (textureAssigned) {


				std::string resolution = "Resolution: " + std::to_string(activeMat.GetMetallicTexture().GetWidth()) + "x" + std::to_string(activeMat.GetMetallicTexture().GetHeight());
				ImGui::Text(resolution.c_str());

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


			ImGui::TreePop();
		} // End Metallic Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Ambient Occlusion Settings
		if (ImGui::TreeNodeEx("Ambient Occlusion ##Settings", m_textureTreeNodeFlags))
		{
			//ImGui::SeparatorText("Ambient Occlusion ");
			bool textureAssigned = activeMat.isAoAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##AO", m_buttonSizeLoad)) {
				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_AMBIENTOCCLUSION, filepath);
					SaveMaterial();
				}
			}
			m_isAmbientOcclusionButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Delete ##AO", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_AMBIENTOCCLUSION);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			if (textureAssigned) {


				std::string resolution = "Resolution: " + std::to_string(activeMat.GetAOTexture().GetWidth()) + "x" + std::to_string(activeMat.GetAOTexture().GetHeight());
				ImGui::Text(resolution.c_str());

			}

			ImGui::TreePop();
		} // End Ambient Occlusion Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Emission Settings
		if (ImGui::TreeNodeEx("Emission ##Settings", m_textureTreeNodeFlags)) {

			bool textureAssigned = activeMat.isEmissiveAssigned();

			// Load Texture
			if (ImGui::Button("Load Texture...##Emissive", m_buttonSizeLoad)) {
				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_EMISSION, filepath);
					SaveMaterial();

				}
			}
			m_isEmissionButtonHovered = ImGui::IsItemHovered();


			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Delete ##Emissive", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_EMISSION);
					textureAssigned = false;
					SaveMaterial();
				}
			}

			if (textureAssigned) {

				std::string resolution = "Resolution: " + std::to_string(activeMat.GetEmissiveTexture().GetWidth()) + "x" + std::to_string(activeMat.GetEmissiveTexture().GetHeight());
				ImGui::Text(resolution.c_str());
			}




			if (!textureAssigned)
				ImGui::BeginDisabled();

			{

				bool useEmitAsMask = activeMat.UseEmissiveAsMask;
				ImGui::Checkbox("Use Emissive As Mask", &useEmitAsMask);
				gui::GuiProcedures::SamelineTooltip("Use this if emission texture is a grayscale mask\nand you want to specify color seperatly.");

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


			ImGui::TreePop();
		} // End Emission Settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Height Settings
		if (ImGui::TreeNodeEx("Height ##Settings", m_textureTreeNodeFlags))
		{
			//ImGui::SeparatorText("Height ");
			bool textureAssigned = activeMat.isHeightAssigned();


			// Load Texture
			if (ImGui::Button("Load Texture...##Height", m_buttonSizeLoad)) {

				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);

				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_HEIGHT, filepath);
					SaveMaterial();
				}
			}

			m_isHeightButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Delete ##Height", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_HEIGHT);
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

				if (ImGui::SliderFloat("Max Height", &activeMat.MaxHeight, 0.0f, 1.0f, "%.4f")) {
					m_valuesChanged = true;
				}
				ImGui::SetItemTooltip("Specify how bright the brightes pixel in the source texture is.\nUsefull to offset the depth rendering.");

			}
			if (!textureAssigned)
				ImGui::EndDisabled();


			ImGui::TreePop();

		} // End Hight Settings


		ImGui::Spacing();
		ImGui::Spacing();

		// Opacity Settings
		if (ImGui::TreeNodeEx("Opacity ##Settings", m_textureTreeNodeFlags)) {


			//ImGui::SeparatorText("Opacity ");

			bool textureAssigned = activeMat.isOpacityAssigned();


			// Load Texture
			if (ImGui::Button("Load Texture...##Opacity", m_buttonSizeLoad)) {

				fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);

				if (!filepath.empty()) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_OPACITY, filepath);
					SaveMaterial();
				}
			}

			m_isOpacityButtonHovered = ImGui::IsItemHovered();

			if (textureAssigned) {
				// Remove Texture
				ImGui::SameLine();
				if (ImGui::Button("Delete ##Opacity", m_buttonSizeDelete)) {
					m_materialRegistry.ActiveLibEntry_PbrMat_DeleteTexture(graphics::MNSY_TEXTURE_OPACITY);
					textureAssigned = false;
					SaveMaterial();
				}
	
			}


			if (!textureAssigned && activeMat.isAlbedoAssigned()) {

				if (ImGui::Button("Generate from albedo alpha", ImVec2(230, 0))) {

					m_materialRegistry.ActiveLibEntry_PbrMat_GenerateOpacityFromAlbedoAlpha(activeLibEntry, activeMat);
					SaveMaterial();
				}
				gui::GuiProcedures::SamelineTooltip("Generate the Opacity map from Albedo map alpha channel");

			}


			//m_isOpacityButtonHovered = ImGui::IsItemHovered();


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

				bool useDithered = activeMat.UseDitheredAlpha;
				ImGui::Checkbox("Dithered Alpha Clip", &useDithered);
				
				if(useDithered != activeMat.UseDitheredAlpha) {
					activeMat.UseDitheredAlpha = useDithered;
					m_valuesChanged = true;
				}


				if (ImGui::SliderFloat("Opacity Threshold", &activeMat.OpacityTreshhold, 0.0f, 1.0f, "%.4f")) {
					m_valuesChanged = true;
				}

			}
			if (!textureAssigned)
				ImGui::EndDisabled();


			ImGui::TreePop();

		} // End Opacity Settings

		ImGui::Spacing();
		ImGui::Spacing();



	}

	void MaterialEditorGuiPanel::DrawPbrMatChannelPackUI(systems::LibEntry* activeLibEntry, graphics::PbrMaterial& activeMat) {
		
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::SeparatorText("Channel Packing");


		ImGui::Spacing();
		ImGui::Spacing();


		// Show a list of all already packed textures

		if (ImGui::TreeNodeEx("Packed Textures", ImGuiTreeNodeFlags_DefaultOpen)) {

			if (activeMat.HasPackedTextures) {

				if (!activeMat.PackedTexturesSuffixes.empty()) { // just to be safe

					for (int i = 0; i < activeMat.PackedTexturesSuffixes.size(); i++) {

						std::string name = " - " + activeLibEntry->name + activeMat.PackedTexturesSuffixes[i];
						ImGui::Selectable(name.c_str());

						// delete packed texture right click menu
						if (ImGui::BeginPopupContextItem()) {
						
							if (ImGui::Selectable("Delete packed texture")) {

								m_materialRegistry.ActiveLibEntry_PbrMat_DeleteChannelPackedTexture(activeLibEntry,activeMat, activeMat.PackedTexturesSuffixes[i]);
							}

							ImGui::EndPopup();
						}
					}
				}
			}
			else {
				ImGui::Text("There are no packed textures for this material yet");
			}
		
			ImGui::TreePop();
		}

		// Create new channel packed texture
		if (ImGui::TreeNodeEx("Create Packed Texture")) {


			// input field to name the extention
			static std::string fileSuffix = "_suffix";

			{

				ImGui::Text("File Suffix: ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::InputText("##RenameMaterialInputField", &fileSuffix, m_textInputFlags);
			

				std::string fileNamePreview = "Filename: " + activeLibEntry->name + fileSuffix;

				ImGui::Text(fileNamePreview.c_str());
			}

			ImGui::Spacing();
			ImGui::Spacing();

			// resolution
			static int resolutionX = 1024;
			static int resolutionY = 1024;
			ImGui::Text("Resolution X: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120);
			ImGui::DragInt(" ##ResX",&resolutionX,1.0f,4,8192);
			
			ImGui::Text("Resolution Y: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120);
			ImGui::DragInt(" ##ResY",&resolutionY,1.0f,4,8192);

			// pack type 
			static int current_packtype = 1;

			ImGui::Text("Pack Fromat: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150);
			ImGui::Combo(" ##PckFmt", &current_packtype, graphics::TexDefinitions::ChannelPackTypes_string, IM_ARRAYSIZE(graphics::TexDefinitions::ChannelPackTypes_string));

			static int current_bitDepth = 1;
			ImGui::Text("Bit Depth: ");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150);
			ImGui::Combo(" ##BitDepth", &current_bitDepth, graphics::TexDefinitions::BitDepthTypes_string, IM_ARRAYSIZE(graphics::TexDefinitions::BitDepthTypes_string));


			// select pack components for each channel of pack type
			ImGui::Spacing();
			ImGui::Spacing();


			static int curr_packComponent_R = 1;
			static int curr_packComponent_G = 2;
			static int curr_packComponent_B = 3;
			static int curr_packComponent_A = 14;

			ImGui::Text("R - Channel");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			ImGui::Combo(" ## R_channel", &curr_packComponent_R, graphics::TexDefinitions::ChannelPackComponents_string, IM_ARRAYSIZE(graphics::TexDefinitions::ChannelPackComponents_string));

			ImGui::Text("G - Channel");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			ImGui::Combo(" ## G_channel", &curr_packComponent_G, graphics::TexDefinitions::ChannelPackComponents_string, IM_ARRAYSIZE(graphics::TexDefinitions::ChannelPackComponents_string));

			
			ImGui::Text("B - Channel");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			ImGui::Combo(" ## B_channel", &curr_packComponent_B, graphics::TexDefinitions::ChannelPackComponents_string, IM_ARRAYSIZE(graphics::TexDefinitions::ChannelPackComponents_string));
			
			
			if ((graphics::ChannelPackType)current_packtype == graphics::ChannelPackType::MNSY_PACKTYPE_RGBA) {
				
				ImGui::Text("A - Channel");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::Combo(" ## A_channel", &curr_packComponent_A, graphics::TexDefinitions::ChannelPackComponents_string, IM_ARRAYSIZE(graphics::TexDefinitions::ChannelPackComponents_string));
			}

			ImGui::Spacing();
			ImGui::Spacing();


			if (ImGui::Button("Create Packed Texture")) {

				graphics::ChannelPackType packType = (graphics::ChannelPackType)current_packtype;
				graphics::ChannelPackComponent R = (graphics::ChannelPackComponent)curr_packComponent_R;
				graphics::ChannelPackComponent G = (graphics::ChannelPackComponent)curr_packComponent_G;
				graphics::ChannelPackComponent B = (graphics::ChannelPackComponent)curr_packComponent_B;
				graphics::ChannelPackComponent A = (graphics::ChannelPackComponent)curr_packComponent_A;

				int bitDepth = 16;
				if (current_bitDepth == 0) {
					bitDepth = 8;
				}
				else if (current_bitDepth == 2) {
					bitDepth = 32;
				}

				m_materialRegistry.ActiveLibEntry_PbrMat_GenerateChannelPackedTexture(activeLibEntry,activeMat,fileSuffix, packType,R,G,B,A,resolutionX,resolutionY,bitDepth);
			}


			ImGui::TreePop();
		}
	}

	void MaterialEditorGuiPanel::DrawEntryUnlitMat(systems::LibEntry* activeLibEntry, graphics::UnlitMaterial* unlitMat) {
		
		MNEMOSY_ASSERT(unlitMat != nullptr, "This should never happen!");


		// import & material settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Load Texture
		if (ImGui::Button("Load Texture...##UnlitMat", m_buttonSizeLoad)) {

			fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
			if (!filepath.empty()) {
				m_materialRegistry.ActiveLibEntry_UnlitMat_LoadTexture(filepath);
				SaveMaterial();
			}
		}
		m_isUnlitLoadButtonHovered = ImGui::IsItemHovered();


		bool textureAssigned = unlitMat->TextureIsAssigned();

		// we cant merge this with the below if statement because the button can change the state of the texture assigned bool
		if (textureAssigned) {
			// Remove Texture
			ImGui::SameLine();

			/*ImGui::PushID("Remove ##UnlitTex");

			ImGui::PushFont(MnemosyEngine::GetInstance().GetUserInterface().Font_Icon_Get());

			static const std::string removeTxt = core::StringUtils::wChar_to_utf8String_appendable(0xE92B) + " Delete";

			bool removePressed = ImGui::Button(removeTxt.c_str(), m_buttonSizeDelete);
			
			ImGui::PopFont();

			ImGui::PopID();*/


			if (ImGui::Button("Delete ##UnlitTex", m_buttonSizeDelete)) {

				m_materialRegistry.ActiveLibEntry_UnlitMat_DeleteTexture();
				textureAssigned = false;
				SaveMaterial();
			}
		}


		ImGui::Spacing();
		ImGui::Spacing();


		float uvTiling[2] = {unlitMat->UVTilingX,unlitMat->UVTilingY};
		if (ImGui::DragFloat2("UV Tiling", (float*)uvTiling, 0.1f, 0.0, 1000, "%0.4f")) {

			unlitMat->UVTilingX = uvTiling[0];
			unlitMat->UVTilingY = uvTiling[1];

			m_valuesChanged = true;
		}


		// write info
		if (textureAssigned) {

			std::string resolution = "Resolution: " + std::to_string(unlitMat->GetTexture().GetWidth()) + "x" + std::to_string(unlitMat->GetTexture().GetHeight() ) + "px";
			ImGui::Text(resolution.c_str());
		}


		bool useAlpha = unlitMat->useAlpha;
		ImGui::Checkbox("Use Alpha Clip", &useAlpha);

		if (useAlpha != unlitMat->useAlpha) {
			unlitMat->useAlpha = useAlpha;
			m_valuesChanged = true;
		}


		// if not using alpha clip disable these widgets
		if (!useAlpha)
			ImGui::BeginDisabled();
		{


			bool useDitheredAlpha = unlitMat->useDitheredAlpha;
			ImGui::Checkbox("Use Dithered Alpha Clip", &useDitheredAlpha);

			if (useDitheredAlpha != unlitMat->useDitheredAlpha) {
				unlitMat->useDitheredAlpha = useDitheredAlpha;
				m_valuesChanged = true;
			}

			if (ImGui::SliderFloat("Alpha Threshold", & unlitMat->alphaThreshold, 0.0f, 1.0f, "%.4f")) {
				m_valuesChanged = true;
			}

		}
		if(!useAlpha)
			ImGui::EndDisabled();

	}

	void MaterialEditorGuiPanel::DrawEntrySkybox(systems::LibEntry* activeLibEntry, graphics::Skybox* skyboxMat) {

		//ImGui::Text("Skybox ui is not yet implemented");

		MNEMOSY_ASSERT(skyboxMat != nullptr, "This should never happen!");


		// import & material settings

		ImGui::Spacing();
		ImGui::Spacing();

		// Load Texture
		if (ImGui::Button("Load Texture...##SkyMat", m_buttonSizeLoad)) {

			fs::path filepath = mnemosy::core::FileDialogs::OpenFile(readable_textureFormats_DialogFilter);
			if (!filepath.empty()) {

				std::filesystem::path p = { filepath };
				m_materialRegistry.ActiveLibEntry_Skybox_LoadTexture(p);
				SaveMaterial();
			}
		}
		m_isSkyboxLoadButtonHovered = ImGui::IsItemHovered();


		bool textureAssigned = skyboxMat->HasCubemaps();// IsColorCubeAssigned();

		// we cant merge this with the below if statement because the button can change the state of the texture assigned bool
		if (textureAssigned) {
			// Remove Texture
			ImGui::SameLine();
			if (ImGui::Button("Delete ##Skymat", m_buttonSizeDelete)) {

				m_materialRegistry.ActiveLibEntry_Skybox_DeleteTexture();
				textureAssigned = false;
				SaveMaterial();
			}
		}

		// atm we dont have a propper way of accessing the original resolution because prefitler is constant 1024.
		/*if (textureAssigned) {

			std::string res = std::to_string(skyboxMat->GetPrefilterCube().GetResolution());

			std::string resolution = "Resolution: " + res + "x" + res + "px";
			ImGui::Text(resolution.c_str());


		}*/

		ImGui::Spacing();
		ImGui::Spacing();

		// color,
		if (ImGui::ColorEdit3("Color", (float*)&skyboxMat->color)) {
			m_valuesChanged = true;
		}
		// exposure

		if (ImGui::DragFloat("Exposure", &skyboxMat->exposure,0.01f, -10.0f, 10.0f, "%.5f")) {
			m_valuesChanged = true;
		}

		ImGui::Spacing();
		ImGui::Spacing();


		if (m_valuesChanged) {
			MnemosyEngine::GetInstance().GetRenderer().SetShaderSkyboxUniforms(MnemosyEngine::GetInstance().GetScene().userSceneSettings,*skyboxMat);
		}
	}

	// Callback when files are droped into mnemosy
	void MaterialEditorGuiPanel::OnFileDropInput(int count, std::vector<std::string>& dropedFilePaths) {


		namespace fs = std::filesystem;

		if (!m_materialRegistry.UserEntrySelected())
			return;

		if (!m_isPanelHovered)
			return;
	
		
		if (count == 0)
			return;

		if (dropedFilePaths.empty()) // just making sure
			return;


		systems::LibEntryType activeEntryType = m_materialRegistry.ActiveLibEntry_Get()->type;
		
		// try handle only first one an check if it is above a specific button
		{
			std::filesystem::path firstPath = fs::u8path(mnemosy::core::StringUtils::string_fix_u8Encoding(dropedFilePaths[0]));			

			if (!fs::exists(firstPath)) {
				return;
			}

			if (!fs::is_regular_file(firstPath)) {
				return;
			}

			std::string extention = firstPath.extension().generic_string();



			if (!graphics::TexUtil::is_image_file_extention_supported(extention)) {
				return;
			}

			if (activeEntryType == systems::LibEntryType::MNSY_ENTRY_TYPE_UNLITMAT) {

				// if we drag over the butten it always get loaded even if there is a texture already. if user drags only in the window then we only load the texture if none is loaded already to avoid accidental and unwanted deletions by users.
				if (m_isUnlitLoadButtonHovered) {

					m_materialRegistry.ActiveLibEntry_UnlitMat_LoadTexture(firstPath);
					SaveMaterial();
					return;
				}

				if (!MnemosyEngine::GetInstance().GetScene().GetUnlitMaterial()->TextureIsAssigned()) {
					m_materialRegistry.ActiveLibEntry_UnlitMat_LoadTexture(firstPath);
					SaveMaterial();
					return;
				}
				else {
					MNEMOSY_WARN("A texture is already loaded in the material, to avoid accidental deletions of textures you should delete the texture from the material slot first or drag it directly over the load button.");
				}



			}
			else if (activeEntryType == systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {

				// albedo 
				if (m_isAbedoButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_ALBEDO, firstPath);
					SaveMaterial();
					return;
				}
				// roughness
				else if (m_isRoughnessButtonHovered) {
					
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_ROUGHNESS, firstPath);
					SaveMaterial();
					return;
				}
				// normal
				else if (m_isNormalButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_NORMAL, firstPath);
					SaveMaterial();
					return;
				}
				// metallic
				else if (m_isMetallicButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_METALLIC, firstPath);
					SaveMaterial();
					return;
				}
				// ao
				else if (m_isAmbientOcclusionButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_AMBIENTOCCLUSION, firstPath);
					SaveMaterial();
					return;
				}
				// emission
				else if (m_isEmissionButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_EMISSION, firstPath);
					SaveMaterial();
					return;
				}
				// height
				else if (m_isHeightButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_HEIGHT, firstPath);
					SaveMaterial();
					return;
				}
				// opacity
				else if (m_isOpacityButtonHovered) {
					m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(graphics::MNSY_TEXTURE_OPACITY, firstPath);
					SaveMaterial();
					return;
				}
			}
			else if (activeEntryType == systems::LibEntryType::MNSY_ENTRY_TYPE_SKYBOX) {
				
				// if we drag over the butten it always get loaded even if there is a texture already. if user drags only in the window then we only load the texture if none is loaded already to avoid accidental and unwanted deletions by users.

				if (m_isSkyboxLoadButtonHovered) {

					m_materialRegistry.ActiveLibEntry_Skybox_LoadTexture(firstPath);
					SaveMaterial();
					return;
				}

				// we do the same thing but with out the butten but the only if no texture is assigned yet
				if (!MnemosyEngine::GetInstance().GetScene().GetSkybox().HasCubemaps()) {
					m_materialRegistry.ActiveLibEntry_Skybox_LoadTexture(firstPath);
					SaveMaterial();
					return;
				}
				else {
				
					MNEMOSY_WARN("A texture is already loaded in the material, to avoid accidental deletions of textures you should delete the texture from the material slot first or drag it directly over the load button.");
				
				}
			}		

		}


		if (activeEntryType != systems::LibEntryType::MNSY_ENTRY_TYPE_PBRMAT) {
			return;
		}

		// now see if we can match all filepath into some texture slot
		graphics::PbrMaterial& activeMat = m_engineInstance.GetScene().GetPbrMaterial();


		for (int i = 0; i < dropedFilePaths.size(); i++) {


			std::filesystem::path filepath = fs::u8path(mnemosy::core::StringUtils::string_fix_u8Encoding(dropedFilePaths[i]));

			if(filepath.has_extension()){

				std::string fileExtention = filepath.extension().generic_string();
				if (graphics::TexUtil::is_image_file_extention_supported(fileExtention)) {



					fs::path filename = filepath.filename();

					filename.replace_extension("");

					std::string filenameString = filename.generic_string();

					graphics::PBRTextureType type = graphics::TexUtil::get_PBRTextureType_from_filename(filenameString);



					if (type != graphics::MNSY_TEXTURE_NONE && type != graphics::MNSY_TEXTURE_COUNT) {


						//std::string typeString = graphics::TextureDefinitions::GetTextureNameFromEnumType(type);
						//MNEMOSY_TRACE("FileDrop: {}, Matches Type: {} Path: {}", filenameString,typeString, filepath.generic_string());
						bool textureIsInThisSlot = activeMat.IsTextureTypeAssigned(type);


						if (!textureIsInThisSlot) {
							m_materialRegistry.ActiveLibEntry_PbrMat_LoadTexture(type, filepath);
						} 
						else {

							std::string typeString = graphics::TexUtil::get_string_from_PBRTextureType(type);
							MNEMOSY_WARN("A texture is already loaded in the material slot {}, to avoid accidental deletions of textures you should delete the texture from the material slot {} first or drag it directly over the respective load button.", typeString, typeString);
						}					
					}
					else {
						MNEMOSY_WARN("Could not determine pbr type for texture with name: {}, {}", filename.generic_string(), filepath.generic_string());					
					}

					SaveMaterial();
				}
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

	void MaterialEditorGuiPanel::SaveMaterial()	{

		m_materialRegistry.ActiveLibEntry_SaveToFile();
		m_valuesChanged = false;
	}

} // !mnemosy::gui
