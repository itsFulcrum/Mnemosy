#ifndef CONTENTS_GUI_PANEL_H
#define CONTENTS_GUI_PANEL_H


#include "Include/Gui/GuiPanel.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

#include <string>

namespace mnemosy::systems {
	class MaterialLibraryRegistry;
	enum LibEntryType;
	struct LibEntry;
	struct FolderNode;
}

namespace mnemosy::graphics {
	class Texture;
}

namespace mnemosy::gui {
	struct LibEntry_Payload;
	struct Folder_Payload;
}


namespace mnemosy::gui {


	class ContentsGuiPanel : public GuiPanel {
	public:
		ContentsGuiPanel();
		~ContentsGuiPanel();

		virtual void Draw() override;

		void ImageButtonSizeSet(float buttonSize) {m_imgButtonSize = buttonSize;}
		float ImageButtonSizeGet() {return m_imgButtonSize;}

	private:


		void DrawMaterialButtons();
		void DrawMaterialButtonsOfSearch();



		void ShortenNameStringToFitButtonSize(std::string& str, float currentImageButtonSize);


		void Folder_DeleteButKeepChildren_Next_Frame(systems::FolderNode* folder, bool isUpdateLoop);
		
		void PopupModal_Folder_DeleteHierachy_Open(systems::FolderNode* folder);
		void PopupModal_Folder_DeleteHierachy();

		void PopupModel_LibEntry_Delete_Open(systems::LibEntry* entry);
		void PopupModal_LibEntry_Delete();

	private:


		systems::MaterialLibraryRegistry& m_materialRegistry;
		std::string m_rename_input_text;
		
		float m_imgButtonSize = 128.0f;

		ImVec2 m_image_button_size = ImVec2(128.0f, 128.0f);

		uint16_t m_currentFolder_id = 0;

		ImGuiInputTextFlags m_textInputFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;

		graphics::Texture* m_folder_icon_tex = nullptr;

		systems::FolderNode* m_folder_deleteNextFrame_ptr = nullptr;
		systems::FolderNode* m_popupModal_folder_deleteHierarchy_pbr = nullptr;
		systems::LibEntry* m_popupModal_libEntry_delete_ptr = nullptr;

		LibEntry_Payload* m_payload_libEntry_ptr = nullptr;
		Folder_Payload* m_payload_folder_ptr = nullptr;


		bool m_popupModal_folder_deleteHierachy_triggered = false;
		bool m_popupModel_LibEntry_delete_triggered = false;

	};

}




#endif // !CONTENTS_GUI_PANEL_H
