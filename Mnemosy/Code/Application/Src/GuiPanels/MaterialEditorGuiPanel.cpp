#include "Application/Include/GuiPanels/MaterialEditorGuiPanel.h"

#include "Application/Include/Application.h"
#include "Engine/Include/Graphics/Scene.h"
#include "Engine/Include/Graphics/Material.h"
#include "Engine/Include/Graphics/RenderMesh.h"
#include "Engine/Include/Graphics/Texture.h"
#include "Engine/External/ImGui/imgui.h"
#include <glm/glm.hpp>

#include "Engine/Include/MnemosyConfig.h"
#ifdef MNEMOSY_PLATFORM_WINDOWS
#include "Engine/Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

namespace mnemosy::gui
{
	
	MaterialEditorGuiPanel::MaterialEditorGuiPanel()
	{
		panelName = "Material Editor";

	}

	void MaterialEditorGuiPanel::Draw()
	{
		if (!showPanel)
			return;


		//graphics::Scene& scene = ENGINE_INSTANCE().GetScene();
		graphics::Material& material = ENGINE_INSTANCE().GetScene().GetMesh().GetMaterial();

		ImGui::Begin(panelName.c_str(), &showPanel);

		{
			ImGui::SeparatorText("Material");


			ImGui::ColorEdit3("Albedo", (float*)&material.Albedo);
			ImGui::SliderFloat("Roughness", &material.Roughness, 0.0f, 1.0f, "%.4f");
			ImGui::SliderFloat("Metallic", &material.Metallic, 0.0f, 1.0f, "%.4f");
			ImGui::ColorEdit3("Emission", (float*)&material.Emission);
			ImGui::DragFloat("Emission Strength", &material.EmissionStrength,0.01f, 0.0f, 10000.0f, "%.4f");


#ifdef MNEMOSY_PLATFORM_WINDOWS

			if (ImGui::Button("Load Albedo..."))
			{
				std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

				if (!filepath.empty())
				{
					material.assignTexture(graphics::ALBEDO, filepath);
				}
			}
			if (ImGui::Button("Load Normal..."))
			{
				std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

				if (!filepath.empty())
				{
					material.assignTexture(graphics::NORMAL, filepath);
				}
			}

			if (ImGui::Button("Load Roughness..."))
			{
				std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

				if (!filepath.empty())
				{
					material.assignTexture(graphics::ROUGHNESS, filepath);
				}
			}

			if (ImGui::Button("Load Metallic..."))
			{
				std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

				if (!filepath.empty())
				{
					material.assignTexture(graphics::METALLIC, filepath);
				}
			}

			if (ImGui::Button("Load Emission..."))
			{
				std::string filepath = mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

				if (!filepath.empty())
				{
					material.assignTexture(graphics::EMISSION, filepath);
				}
			}

#endif

		}


		ImGui::End();

	}



}
