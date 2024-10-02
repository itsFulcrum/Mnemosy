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
		JsonSettings(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription);
		JsonSettings() = default;
		~JsonSettings() = default;

		void SettingsFileOpen(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription);
		void SettingsFileClose(bool& errorCheck,const std::filesystem::path& filepath);

		void SettingsFileWhipe(bool& errorCheck);
		void SettingErase(bool& errorCheck, const std::string& name);


		void SettingsFilePrettyPrintSet(const bool prettyPrint);
		std::string ErrorStringLastGet();


		// write methods
		void SettingWriteBool(bool& errorCheck, const std::string& name, const bool value);
		void SettingWriteInt(bool& errorCheck, const std::string& name, const int value);
		void SettingWriteFloat(bool& errorCheck, const std::string& name, const float value);
		void SettingWriteString(bool& errorCheck, const std::string& name, const std::string& value);

		void SettingWriteVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& value);
		void SettingWriteVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& value);
		void SettingWriteVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& value);
		void SettingWriteVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& value);

		// read methods
		bool SettingReadBool(bool& errorCheck, const std::string& name, const bool defaultValue,const bool writeDefaultIfNotFound);
		int SettingReadInt(bool& errorCheck, const std::string& name, const int defaultValue,const bool writeDefaultIfNotFound);
		float SettingReadFloat(bool& errorCheck, const std::string& name, const float defaultValue,const bool writeDefaultIfNotFound);
		std::string SettingReadString(bool& errorCheck, const std::string& name, const std::string& defaultValue,const bool writeDefaultIfNotFound);

		std::vector<bool> SettingReadVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& defaultValue,const bool writeDefaultIfNotFound);
		std::vector<int> SettingReadVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& defaultValue,const bool writeDefaultIfNotFound);
		std::vector<float> SettingReadVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& defaultValue,const bool writeDefaultIfNotFound);
		std::vector<std::string>  SettingReadVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& defaultValue,const bool writeDefaultIfNotFound);


	private:
		nlohmann::json m_jsonObject;

		bool m_prettyPrint = false;
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


		std::filesystem::path TestFilePath = std::filesystem::path("c:\example\testFile.json");
		
		// almost all methods take a bool for error checking as the first parameter
		// its most important for opening the file bc it may be possible that the file is not correct or corrupted
		// write methods only fail if the files has not been opend
		// if read methods fails they simply return the default value
		
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


		// Write Entries
		bool aBoolean = true;
		settings.SettingWriteBool(success, "testBool", aBoolean);
		
		std::vector<std::string> aVectorOfStrings(4);
		aVectorOfStrings[0]("h");
		aVectorOfStrings[1]("e");
		aVectorOfStrings[2]("y");
		settings.SettingsWriteVectorString(success,"testVector", aVectorOfStrings);

		// Read Entries
		bool writeDefaultValueIfKeyNotFound = true;
		bool defaultValue = false;
		bool readBoolean = settings.SettingReadBool(success,"testBool", defaultValue, writeDefaultValueIfKeyNotFound);

		std::vector<std::string> = settings.SettingReadVectorString(success,"testVector",std::vector<std::string>(),writeDefaultValueIfKeyNotFound);
		
		// Erase Entries
		settings.SettingErase(success, "testBool");
		if(!success){
			// log error if neccesarry 
		}
		
		// Whipe the entire file.
		settings.SettingsFileWhipe(success);



		// optionally set wheather to pretty print or not, default is false.
		settings.SettingsFilePrettyPrintSet(true);

		// At the end close the file.  This is where all the contents actually get written to the file.

		settings.SettingsFileClose(success,TestFilePath);

	}

*/