#ifndef MATERIAL_EDITOR_GUI_PANEL_H
#define MATERIAL_EDITOR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "External/ImGui/imgui.h"
#include "External/ImGui/imgui_stdlib.h"

#include <vector>
#include <string>
#include <filesystem>

namespace mnemosy {
	class MnemosyEngine;
}

namespace mnemosy::systems {
	class MaterialLibraryRegistry;
	class ExportManager;
}

namespace mnemosy::graphics {
	class Material;
}

namespace mnemosy::gui
{
	class MaterialEditorGuiPanel : public GuiPanel
	{
	public:
		MaterialEditorGuiPanel();
		~MaterialEditorGuiPanel();


		virtual void Draw() override;


	private:
		void DrawTextureSettings(graphics::Material& activeMat,std::filesystem::path& libraryDirectory);
		void DrawChannelPackUI(graphics::Material& activeMat);



		void CheckToSaveMaterial(float deltaSeconds);
		void SaveMaterial();

		void OnFileDropInput(int count,std::vector<std::string>& dropedFilePaths);
	private:

		ImVec2 m_buttonSize = ImVec2(120, 0);
		ImVec2 m_buttonSizeLoad = ImVec2(120, 0);
		ImVec2 m_buttonSizeDelete = ImVec2(80, 0);
		ImVec2 m_buttonDrag = ImVec2(120.0f, 45.0f);


		const char* m_normalMapFormats[2] = { "OpenGl", "DirectX" }; // they need to be ordered the same as in material NormalMapFormat Enum
		const char* m_exportFormats[2] = { "tiff","png" }; // they need to be ordered the same as in ExportManager ExportImageFormats

		const char* m_packTypes[2] = { "R+G+B", "R+G+B+A" }; // they need to be orderd the same as ChannelPackType in texture definitions
		// they need to be orderd the same as ChannelPackComponent in texture definitions
		const char* m_packComponents[15] =
		{
			"None",
			"Albedo R",
			"Albedo G",
			"Albedo B",

			"Normal R",
			"Normal G",
			"Normal B",
			
			"Emmissive R",
			"Emmissive G",
			"Emmissive B",

			"Roughness",
			"Metallic",
			"Ambient Occlusion",
			"Height",
			"Opacity"
		};

		ImGuiInputTextFlags m_textInputFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;

		MnemosyEngine& m_engineInstance;
		systems::MaterialLibraryRegistry& m_materialRegistry;
		systems::ExportManager& m_exportManager;

		std::vector<bool> m_exportTexturesBools;
		bool m_exportChannelPackedTexture = true;
		
		ImGuiTreeNodeFlags m_textureTreeNodeFlags;

		int m_onFileDropInput_callback_id = -1;

		int m_currentActiveMaterialID = -1;
		
		bool m_isPanelHovered = false;
		bool m_isAbedoButtonHovered = false;
		bool m_isRoughnessButtonHovered = false;
		bool m_isMetallicButtonHovered = false;
		bool m_isNormalButtonHovered = false;
		bool m_isEmissionButtonHovered = false;
		bool m_isAmbientOcclusionButtonHovered = false;
		bool m_isHeightButtonHovered = false;
		bool m_isOpacityButtonHovered = false;

		bool m_valuesChanged = false;
		float m_TimeToSaveMaterial = 0.5f;
		float m_TimeToSaveMaterialDelta = 0.0f;


	};
}


#endif // !MATERIAL_EDITOR_GUI_PANEL_H
