#include "Include/GuiPanels/DocumentationGuiPanel.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"

#include "ImGui/imgui.h"

#include <filesystem>
#include <fstream>


namespace mnemosy::gui
{
	DocumentationGuiPanel::DocumentationGuiPanel() {
		panelName = "Documentation";
		panelType = MNSY_GUI_PANEL_DOCUMENTATION;

		LoadTextFromFileToString();
	}

	DocumentationGuiPanel::~DocumentationGuiPanel()
	{	}

	void DocumentationGuiPanel::Draw() {

		if (!showPanel)
			return;

		ImGui::Begin(panelName, &showPanel);

		if (ImGui::CollapsingHeader("About Mnemosy")) {

			ImGui::TextWrapped(m_about_text.c_str());
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Geting Started")) {

			ImGui::TextWrapped(m_getStarted_text.c_str());
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Keymap & Controlls")) {

			ImGui::TextWrapped(m_controlls_text.c_str());
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Additional Information")) {

			ImGui::TextWrapped(m_additionalInfo_text.c_str());
		}

		ImGui::End();
	}

	// private
	void DocumentationGuiPanel::LoadTextFromFileToString() {

		std::filesystem::path documentationTextFilesDirectory = MnemosyEngine::GetInstance().GetFileDirectories().GetDocumentationFilesPath();

		std::filesystem::path filePath;

		filePath = documentationTextFilesDirectory / std::filesystem::path("doc_aboutMnemosy.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_about_text);


		filePath = documentationTextFilesDirectory / std::filesystem::path("doc_gettingStarted.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_getStarted_text);

		filePath = documentationTextFilesDirectory / std::filesystem::path("doc_keymapControlls.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_controlls_text);

		filePath = documentationTextFilesDirectory / std::filesystem::path("doc_additionalInformation.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_additionalInfo_text);

	}

	void DocumentationGuiPanel::LoadTextFileToString(const char* filepath, std::string& outString) {

		std::filesystem::directory_entry textFile = std::filesystem::directory_entry(filepath);
		if (!textFile.exists() || textFile.is_directory() || !textFile.is_regular_file() || textFile.path().extension() != ".txt") {

			MNEMOSY_ERROR("DocumentationGuiPanel::LoadTextFileToString: Filepath does not exist or is not a file \nFilepath: {}", filepath);
			outString = "The filepath that contains the text for this string does not exist or is not a file: \nFilepath: " + textFile.path().generic_string();
			return;
		}

		std::ifstream fileContents;
		std::stringstream buffer;

		fileContents.open(filepath);

		buffer << fileContents.rdbuf();
		outString = buffer.str();


		buffer.clear();
		fileContents.close();
	}

} // !mnemosy::gui