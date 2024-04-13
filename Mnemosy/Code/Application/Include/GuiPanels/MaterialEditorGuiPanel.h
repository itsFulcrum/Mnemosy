#ifndef MATERIAL_EDITOR_GUI_PANEL_H
#define MATERIAL_EDITOR_GUI_PANEL_H

#include "Include/Gui/GuiPanel.h"

#include <vector>
#include <string>

namespace mnemosy::systems {
	class MaterialLibraryRegistry;
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

		bool m_isPanelHovered = false;
		bool m_isAbedoButtonHovered = false;
		bool m_isRoughnessButtonHovered = false;
		bool m_isMetallicButtonHovered = false;
		bool m_isNormalButtonHovered = false;
		bool m_isEmissionButtonHovered = false;
		bool m_isAmbientOcclusionButtonHovered = false;

		bool m_isDraggingOnce = false;

		int m_onFileDropInput_callback_id = -1;
		void OnFileDropInput(int count,std::vector<std::string>& dropedFilePaths);

		systems::MaterialLibraryRegistry& m_materialRegistry;
	};
}


#endif // !MATERIAL_EDITOR_GUI_PANEL_H
