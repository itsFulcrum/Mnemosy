#include "Include/GuiPanels/DocumentationGuiPanel.h"

#include "Include/MnemosyEngine.h"
#include "Include/Core/Log.h"
#include "Include/Core/FileDirectories.h"


#include "External/ImGui/imgui.h"

#include <filesystem>
#include <fstream>
#include <string>


namespace mnemosy::gui
{



	DocumentationGuiPanel::DocumentationGuiPanel() {
		panelName = "Documentation";

		LoadTextFromFileToString();
	}

	DocumentationGuiPanel::~DocumentationGuiPanel() {

	}

	void DocumentationGuiPanel::Draw() {

		if (!showPanel)
			return;

		ImGui::Begin(panelName, &showPanel);

		if (ImGui::CollapsingHeader("About Mnemosy")) {

			ImGui::TextWrapped(m_AboutMnemosy_text.c_str());
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Geting Started")) {

			ImGui::TextWrapped(m_GettingStarted_text.c_str());
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Keymap & Controlls")) {

			ImGui::TextWrapped(m_KeyboardControlls_text.c_str());
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Additional Information")) {

			ImGui::TextWrapped(m_AdditionalInformation_text.c_str());
		}


		ImGui::End();
	}

	void DocumentationGuiPanel::LoadTextFromFileToString() {

		std::filesystem::path documentationFileDir = MnemosyEngine::GetInstance().GetFileDirectories().GetDocumentationFilesPath();

		std::filesystem::path filePath;
		filePath = documentationFileDir / std::filesystem::path("doc_aboutMnemosy.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_AboutMnemosy_text);


		filePath = documentationFileDir / std::filesystem::path("doc_gettingStarted.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_GettingStarted_text);

		filePath = documentationFileDir / std::filesystem::path("doc_keymapControlls.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_KeyboardControlls_text);

		filePath = documentationFileDir / std::filesystem::path("doc_additionalInformation.txt");
		LoadTextFileToString(filePath.generic_string().c_str(), m_AdditionalInformation_text);

		
	}

	void DocumentationGuiPanel::LoadTextFileToString(const char* filepath, std::string& outString) {
		
		std::filesystem::directory_entry file = std::filesystem::directory_entry(filepath);
		if (!file.exists() || file.is_directory() || !file.is_regular_file() || file.path().extension() != ".txt") {
						
			MNEMOSY_ERROR("DocumentationGuiPanel::LoadTextFileToString: Filepath does not exist or is not a file \nFilepath: {}", filepath);
			outString = "The filepath that contains the text for this string does not exist or is not a file: \nFilepath: " + file.path().generic_string();
			return;
		}

		std::ifstream fileContent;
		std::stringstream buffer;
		
		fileContent.open(filepath);

		buffer << fileContent.rdbuf();
		outString = buffer.str();

		buffer.clear();
		fileContent.close();
	}

} // !mnemosy::gui