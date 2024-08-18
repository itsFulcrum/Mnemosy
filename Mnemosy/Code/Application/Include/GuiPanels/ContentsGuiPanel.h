#ifndef CONTENTS_GUI_PANEL_H
#define CONTENTS_GUI_PANEL_H


#include "Include/Gui/GuiPanel.h"


namespace mnemosy::systems {
	class MaterialLibraryRegistry;
}

namespace mnemosy::gui {


	class ContentsGuiPanel : public GuiPanel {
	public:
		ContentsGuiPanel();
		~ContentsGuiPanel();

		virtual void Draw() override;

	private:
		void DrawMaterialButtons();
		void DrawMaterialButtonsOfSearch();

		systems::MaterialLibraryRegistry& m_materialRegistry;
		float m_imgButtonSize = 128.0f;
	};

}




#endif // !CONTENTS_GUI_PANEL_H
