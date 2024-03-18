#ifndef MATERIAL_EDITOR_GUI_PANEL_H
#define MATERIAL_EDITOR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

namespace mnemosy::systems {
	class MaterialLibraryRegistry;
}

namespace mnemosy::gui
{
	class MaterialEditorGuiPanel : public GuiPanel
	{
	public:
		MaterialEditorGuiPanel();

		virtual void Draw() override;

	private:
		systems::MaterialLibraryRegistry& m_materialRegistry;
	};
}


#endif // !MATERIAL_EDITOR_GUI_PANEL_H
