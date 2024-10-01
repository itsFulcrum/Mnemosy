#ifndef JSON_SETTINGS_H
#define JSON_SETTINGS_H

// HOW TO USE AT THE BOTTOM OF THE FILE

#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace mnemosy::core {

	class JsonSettings {

	public:
		JsonSettings() = default;
		~JsonSettings() = default;

		void SettingsFileOpen(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription);
		void SettingsFileClose(bool& errorCheck,const std::filesystem::path& filepath);

		void SettingsFileWhipe(bool& errorCheck);
		void SettingErase(bool& errorCheck, const std::string& name);


		void SettingsFilePrettyPrintSet(const bool prettyPrint);
		std::string ErrorStringLastGet();




		// write methods
		// FIXME: you can technically write to the json object before opening a file..
		void SettingWriteBool(bool& errorCheck, const std::string& name, const bool value);
		void SettingWriteInt(bool& errorCheck, const std::string& name, const int value);
		void SettingWriteFloat(bool& errorCheck, const std::string& name, const float value);
		void SettingWriteString(bool& errorCheck, const std::string& name, const std::string& value);

		void SettingWriteVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& value);
		void SettingWriteVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& value);
		void SettingWriteVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& value);
		void SettingWriteVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& value);

		// read methods
		bool SettingReadBool(bool& errorCheck, const std::string& name, const bool defaultValue);
		int SettingReadInt(bool& errorCheck, const std::string& name, const int defaultValue);
		float SettingReadFloat(bool& errorCheck, const std::string& name, const float defaultValue);
		std::string SettingReadString(bool& errorCheck, const std::string& name, const std::string& defaultValue);

		std::vector<bool> SettingReadVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& defaultValue);
		std::vector<int> SettingReadVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& defaultValue);
		std::vector<float> SettingReadVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& defaultValue);
		std::vector<std::string>  SettingReadVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& defaultValue);


	private:
		nlohmann::json m_jsonObject;

		bool m_prettyPrint = true;
		bool m_fileIsOpen = false;
		std::string m_lastErrorString = "none";

	};

} // ! namespace mnemosy::core

#endif // ! JSON_SETTINGS_H


// How to use
/*	

	void example(){
	
		// create a settings object
		mnemosy::core::JsonSettings settings;
		
		// optionally set wheather to pretty print or not.
		settings.SettingsFilePrettyPrintSet(true);


		std::filesystem::path TestFilePath = std::filesystem::path("c:\example\testFile.json");
		
		// almost all methods take a bool for error checking as the first parameter
		bool success = false;
	
		// first open the file
		settings.SettingsFileOpen(success, TestFilePath, std::string("HeaderGoesHere"),std::string("This is a test file"));
		if(!success){
			// if any error check fails you can receive an error string of the last thrown error with settings.ErrorStringLastGet();
			std::cout << "Failed to open settings file. message: " << settings.ErrrorStringLastGet() << "\n";
			return;
		}
		
	


		//settings.SettingsFileWhipe(success);

		// settings.SettingErase(success, "TestVectorInt");
		// if(!success){
		// 	MNEMOSY_WARN("Faild to erase setting:  {}", settings.ErrorStringLastGet());
		// }



		// Read And Write to the file while its open

		bool aBoolean = true;
		settings.SettingWriteBool(success, "testBool", aBoolean);
		

		std::vector<std::string> aVectorOfStrings(4);
		aVectorOfStrings[0]("h");
		aVectorOfStrings[1]("e");
		aVectorOfStrings[2]("y");
		settings.SettingsWriteVectorString(success,"testVector", aVectorOfStrings);


		bool defaultValue = false;
		settings.SettingReadBool(success,"testBool", defaultValue);


		settings.SettingReadVectorString(success,"testVector",std::vector<std::string>());

		
		// to erase an entry use

		settings.SettingErase(success, "testBool");
		if(!success){
			// log error if neccesarry 
		}
		
		// or whipe the entire file.
		settings.SettingsFileWhipe(success);

		

		// at the end close the file.  this is where all the contents actually get written to the file.

		settings.SettingsFileClose(success,TestFilePath);

	}

*/