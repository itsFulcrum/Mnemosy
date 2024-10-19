#ifndef JSON_SETTINGS_H
#define JSON_SETTINGS_H

// HOW TO USE AT THE BOTTOM OF THE FILE

#include <filesystem>
#include <string>
#include <vector>

#include <json.hpp>

namespace mnemosy::core {

	class JsonSettings {

	public:
		JsonSettings(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription);
		JsonSettings() = default;
		~JsonSettings() = default;

		void FileOpen(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription);
		void FileClose(bool& errorCheck,const std::filesystem::path& filepath);
		bool FileIsOpen() { return m_fileIsOpen; }
		void FileWhipe(bool& errorCheck);
		void Entry_Erase(bool& errorCheck, const std::string& name);


		void FilePrettyPrintSet(const bool prettyPrint);
		std::string ErrorStringLastGet();


		// write methods
		void WriteBool(bool& errorCheck, const std::string& name, const bool value);
		void WriteInt(bool& errorCheck, const std::string& name, const int value);
		void WriteFloat(bool& errorCheck, const std::string& name, const float value);
		void WriteString(bool& errorCheck, const std::string& name, const std::string& value);

		void WriteVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& value);
		void WriteVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& value);
		void WriteVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& value);
		void WriteVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& value);

		// read methods
		bool ReadBool(bool& errorCheck, const std::string& name, const bool defaultValue,const bool writeDefaultIfNotFound);
		int ReadInt(bool& errorCheck, const std::string& name, const int defaultValue,const bool writeDefaultIfNotFound);
		float ReadFloat(bool& errorCheck, const std::string& name, const float defaultValue,const bool writeDefaultIfNotFound);
		std::string ReadString(bool& errorCheck, const std::string& name, const std::string& defaultValue,const bool writeDefaultIfNotFound);

		std::vector<bool> ReadVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& defaultValue,const bool writeDefaultIfNotFound);
		std::vector<int> ReadVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& defaultValue,const bool writeDefaultIfNotFound);
		std::vector<float> ReadVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& defaultValue,const bool writeDefaultIfNotFound);
		std::vector<std::string>  ReadVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& defaultValue,const bool writeDefaultIfNotFound);


	private:
		nlohmann::json m_jsonObject;

		bool m_prettyPrint = false;
		bool m_fileIsOpen = false;
		std::string m_lastErrorString = "none";

	};

} // ! namespace mnemosy::core

#endif // ! JSON_SETTINGS_H


// How to use

// JsonSettings is intended as a failsafe an quick way to read/write data to disk. 
// It uses nlohmann::json internally and only creates a single json object inside the file that contains everything.
// all read methods take a default value that is returned if the file or the entry doesn't exist yet. 

// Quick note. if you only want to read data it is not strictly nessesary to close the file. 
// the file is actually closed already in FileOpen(), however the data will persits in the internally loaded json object untils the JsonSettings object goes out of scope
// when anything was changed for example if for ReadMethods the writeDefaultValueIfKeyNotFound is set then the file has to be closed to update the data in the file.

/*	

	void example(){
	
		// create a settings object
		mnemosy::core::JsonSettings settings;


		std::filesystem::path TestFilePath = std::filesystem::path("c:\example\testFile.json");
		
		// almost all methods take a bool for error checking as the first parameter
		// its most important for opening the file bc it may be possible that the file is not correct or corrupted
		// write methods only fail if the files has not been opend
		// if read methods fails they simply return the default value
		// if an error occures the last thrown error message will be returned by settings.ErrorStringLastGet();
		// most of the time it should be save to use without error checking only if files are corruped or the entire folder is missing the filesystem may throw an error

		bool success = false;
	
		// first open the file
		settings.FileOpen(success, TestFilePath, std::string("HeaderGoesHere"),std::string("This is a test file"));
		if(!success){
			// if any error check fails you can receive an error string of the last thrown error with settings.ErrorStringLastGet();
			std::cout << "Failed to open settings file. message: " << settings.ErrrorStringLastGet() << "\n";
			return;
		}


		// Write Entries
		bool aBoolean = true;
		settings.WriteBool(success, "testBool", aBoolean);
		
		std::vector<std::string> aVectorOfStrings(3);
		aVectorOfStrings[0]("h");
		aVectorOfStrings[1]("e");
		aVectorOfStrings[2]("y");
		settings.WriteVectorString(success,"testVector", aVectorOfStrings);

		// Read Entries
		bool writeDefaultValueIfKeyNotFound = true;
		bool defaultValue = false;
		bool readBoolean = settings.ReadBool(success,"testBool", defaultValue, writeDefaultValueIfKeyNotFound);

		// if this fails the empty vector will be returned. but you can populated it with default values if you need to always get something
		std::vector<std::string> = settings.ReadVectorString(success,"testVector",std::vector<std::string>(),writeDefaultValueIfKeyNotFound);
		
		// Erase Entries
		settings.EntryErase(success, "testBool");
		if(!success){
			// log error if neccesarry 
		}
		
		// Whipe the entire file.
		settings.FileWhipe(success);



		// optionally set wheather to pretty print or not, default is false.
		settings.FilePrettyPrintSet(true);

		// At the end close the file.  This is where all the contents actually get written to the file.

		settings.FileClose(success,TestFilePath);

	}

*/