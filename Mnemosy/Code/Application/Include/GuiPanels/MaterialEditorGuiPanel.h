#ifndef MATERIAL_EDITOR_GUI_PANEL_H
#define MATERIAL_EDITOR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

#include <vector>
#include <string>
#include <filesystem>

namespace mnemosy {
	class MnemosyEngine;
}

namespace mnemosy::systems {
	class MaterialLibraryRegistry;
	class ExportManager;
	struct LibEntry;
}

namespace mnemosy::graphics {
	class UnlitMaterial;
	class PbrMaterial;
	class Skybox;
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

		void DrawExportSettings(systems::LibEntry* activeLibEntry);

		void DrawPbrMaterial(systems::LibEntry* activeLibEntry, graphics::PbrMaterial& activeMat);
		void DrawPbrMatTextureSettings(systems::LibEntry* activeLibEntry, graphics::PbrMaterial& activeMat,std::filesystem::path& libraryDirectory);
		void DrawPbrMatChannelPackUI(systems::LibEntry* activeLibEntry, graphics::PbrMaterial& activeMat);

		void DrawEntryUnlitMat(systems::LibEntry* activeLibEntry, graphics::UnlitMaterial* unlitMat);

		void DrawEntrySkybox(systems::LibEntry* activeLibEntry, graphics::Skybox* skybox);


		void CheckToSaveMaterial(float deltaSeconds);
		void SaveMaterial();

		void OnFileDropInput(int count,std::vector<std::string>& dropedFilePaths);
	private:

		

		ImVec2 m_buttonSize = ImVec2(120, 0);
		ImVec2 m_buttonSizeLoad = ImVec2(120, 0);
		ImVec2 m_buttonSizeDelete = ImVec2(80, 0);
		ImVec2 m_buttonDrag = ImVec2(120.0f, 45.0f);

		ImGuiInputTextFlags m_textInputFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;

		MnemosyEngine& m_engineInstance;
		systems::MaterialLibraryRegistry& m_materialRegistry;
		systems::ExportManager& m_exportManager;

		std::vector<bool> m_pbrMat_exportTextureBools;
		bool m_pbrMat_exportChannelPackedTexture = true;
		
		ImGuiTreeNodeFlags m_textureTreeNodeFlags;

		int m_onFileDropInput_callback_id = -1;

		int m_lastActiveLibEntry_ID = -1;
		
		bool m_isPanelHovered = false;
		bool m_isAbedoButtonHovered = false;
		bool m_isRoughnessButtonHovered = false;
		bool m_isMetallicButtonHovered = false;
		bool m_isNormalButtonHovered = false;
		bool m_isEmissionButtonHovered = false;
		bool m_isAmbientOcclusionButtonHovered = false;
		bool m_isHeightButtonHovered = false;
		bool m_isOpacityButtonHovered = false;

		bool m_isUnlitLoadButtonHovered = false;

		bool m_isSkyboxLoadButtonHovered = false;

		bool m_valuesChanged = false;
		float m_TimeToSaveMaterial = 0.5f;
		float m_TimeToSaveMaterialDelta = 0.0f;


	};
}


#endif // !MATERIAL_EDITOR_GUI_PANEL_H
