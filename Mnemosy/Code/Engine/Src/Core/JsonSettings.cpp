#include "Include/Core/JsonSettings.h"

#include <fstream>

namespace mnemosy::core
{

	JsonSettings::JsonSettings(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription){

		FileOpen(errorCheck, filepath, headerName,fileDescription);
	}

	void JsonSettings::FileOpen(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription){

		namespace fs = std::filesystem;

		errorCheck = true;

		if(m_fileIsOpen){

			errorCheck = false;
			m_lastErrorString = "file is already open - it has to be closed first";
			return;
		}

		if (!filepath.has_extension()) {
			errorCheck = false;
			m_lastErrorString = "filepath is pointing to a folder not a file: " + filepath.generic_string();
			m_fileIsOpen = false;
			return;
		}

		fs::directory_entry settingsFile;

		try {
			settingsFile = fs::directory_entry(filepath);
		}
		catch (fs::filesystem_error err) {
			errorCheck = false;
			m_lastErrorString = "Faild to initialze direcotry entry: " + std::string(err.what());
			m_fileIsOpen = false;
			return;
		}

		// if the parent folder of the filepath provided does not exist we create it here.
		if (!settingsFile.exists()) {

			// we already check that the filepath has a valid extention so here we check wheather the parent path exists, if not we create the directories
			fs::path folder = filepath.parent_path();

			fs::directory_entry parentFolder = fs::directory_entry(folder);

			if (!parentFolder.exists()) {

				try {
					fs::create_directories(folder);
				}
				catch (fs::filesystem_error err) {
					errorCheck = false;
					m_lastErrorString = "provided filepath folders did not exist. atempting to create those folders faild: \nFilesystem error:" + std::string(err.what());
					m_fileIsOpen = false;
					return;
				}
			}
		}


		// if file doesn't exist yet create it
		if (!settingsFile.exists() || !settingsFile.is_regular_file()) {

			m_jsonObject["1_Header"] = headerName;
			m_jsonObject["2_Description"] = fileDescription;

			std::ofstream outFileStream;
			outFileStream.open(filepath);

			if(m_prettyPrint){
				outFileStream << m_jsonObject.dump(4);
			}else{
				outFileStream << m_jsonObject.dump(-1);
			}

			outFileStream.close();
			
			m_fileIsOpen = true;
			return;
		}

		// otherwise read in the json file

		std::ifstream inputFileStream;
		inputFileStream.open(filepath);

		try {
			m_jsonObject = nlohmann::json::parse(inputFileStream);
		} catch (nlohmann::json::parse_error err){

			errorCheck = false;
			m_fileIsOpen = true;
			m_lastErrorString = "Faild to read json file - invalid json contents? File will be cleared. Message:" + std::string(err.what());
			inputFileStream.close();

			// if we fail to read in the filecontents for example if the file contents are not valid json,
			// we clear everything and start with an empty json file.
			// clear the file but 
			m_jsonObject.clear();
		}

		inputFileStream.close();

		// header and description should always be set, if file was whiped for example they would dissapear bec its still a valid json file
		m_jsonObject["1_Header"] = headerName;
		m_jsonObject["2_Description"] = fileDescription;
		
		m_fileIsOpen = true;

	}

	void JsonSettings::FileClose(bool& errorCheck, const std::filesystem::path& filepath){

		errorCheck = true;

		if(!m_fileIsOpen)
		{
			errorCheck = false;
			m_lastErrorString = "No file has been opend yet.";
			return;
		}


		std::ofstream outFileStream;
		outFileStream.open(filepath);

		if(m_prettyPrint){
			outFileStream << m_jsonObject.dump(4);
		}
		else{
			outFileStream << m_jsonObject.dump(-1);
		}

		outFileStream.close();

		m_jsonObject.clear();
		m_jsonObject = nlohmann::json();
	}

	void JsonSettings::FileWhipe(bool& errorCheck){

		errorCheck = true;
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "cannot whipe file because it hasn't been opend yet";
			return;
		}

		m_jsonObject.clear();

	}

	void JsonSettings::Entry_Erase(bool& errorCheck, const std::string& name){

		errorCheck = true;

		if(!m_jsonObject.contains(name)){
			errorCheck = false;
			m_lastErrorString = std::string("file does not contain the key: " + name + " to erase it.");
			return;
		}

		try {
			m_jsonObject.erase(m_jsonObject.find(name));
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
			return;
		}

	}	

	void JsonSettings::FilePrettyPrintSet(const bool prettyPrint){
		m_prettyPrint = prettyPrint;
	}

	std::string JsonSettings::ErrorStringLastGet(){
		return m_lastErrorString;
	}


// Write methods
	void JsonSettings::WriteBool(bool& errorCheck, const std::string& name, const bool value){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::WriteInt(bool& errorCheck, const std::string& name, const int value) {
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::WriteFloat(bool& errorCheck, const std::string& name, const float value) {
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}
	
	void JsonSettings::WriteString(bool& errorCheck, const std::string& name, const std::string& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}


	void JsonSettings::WriteVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::WriteVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::WriteVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::WriteVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

// Read Methods
	bool JsonSettings::ReadBool(bool& errorCheck, const std::string& name, const bool defaultValue,const bool writeDefaultIfNotFound){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}


		errorCheck = true;
		bool output = defaultValue;

		if(!m_jsonObject.contains(name)) {

			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}

			return output;
		}

		if(!m_jsonObject[name].is_boolean()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<bool>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

	int JsonSettings::ReadInt(bool& errorCheck, const std::string& name, const int defaultValue,const bool writeDefaultIfNotFound){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		int output = defaultValue;

		if(!m_jsonObject.contains(name)) {

			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_number_integer()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<int>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}
	

	float JsonSettings::ReadFloat(bool& errorCheck, const std::string& name, const float defaultValue,const bool writeDefaultIfNotFound){
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		float output = defaultValue;

		if(!m_jsonObject.contains(name)) {

			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_number_float()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<float>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

	std::string JsonSettings::ReadString(bool& errorCheck, const std::string& name, const std::string& defaultValue,const bool writeDefaultIfNotFound) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::string output = defaultValue;

		if(!m_jsonObject.contains(name)) {

			if(writeDefaultIfNotFound){

				m_jsonObject[name] = defaultValue;
			}else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_string()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<std::string>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

	std::vector<bool> JsonSettings::ReadVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& defaultValue,const bool writeDefaultIfNotFound) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<bool> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			
			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_array()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<std::vector<bool>>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

	std::vector<int> JsonSettings::ReadVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& defaultValue,const bool writeDefaultIfNotFound) {
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<int> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_array()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<std::vector<int>>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

	std::vector<float> JsonSettings::ReadVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& defaultValue,const bool writeDefaultIfNotFound){
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<float> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_array()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<std::vector<float>>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

	std::vector<std::string> JsonSettings::ReadVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& defaultValue,const bool writeDefaultIfNotFound){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<std::string> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			if(writeDefaultIfNotFound){
				m_jsonObject[name] = defaultValue;
			}
			else{
				errorCheck = false;
				m_lastErrorString = "the given key does not exist yet - returning default";
			}
			return output;
		}

		if(!m_jsonObject[name].is_array()) {
			errorCheck = false;
			m_lastErrorString = "value does not have the correct type - returning default";
			return output;
		}

		try {
			output = m_jsonObject[name].get<std::vector<std::string>>();
		}
		catch (const nlohmann::json::exception& e) {
			errorCheck = false;
			m_lastErrorString = e.what();
		} 

		return output;
	}

// private methods


} // ! namespace mnemosy::core
