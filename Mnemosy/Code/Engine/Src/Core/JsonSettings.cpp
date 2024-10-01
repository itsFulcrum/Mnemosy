#include "Include/Core/JsonSettings.h"

#include <fstream>

namespace mnemosy::core
{

	void JsonSettings::SettingsFileOpen(bool& errorCheck, const std::filesystem::path& filepath, const std::string& headerName, const std::string& fileDescription){

		namespace fs = std::filesystem;

		errorCheck = true;

		if(m_fileIsOpen){

			errorCheck = false;
			m_lastErrorString = "file is already open - it has to be closed first";
			return;
		}


		fs::directory_entry settingsFile = fs::directory_entry(filepath);

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
			m_fileIsOpen = false;
			m_lastErrorString = err.what();
			inputFileStream.close();
			return;
		}

		inputFileStream.close();

		// header and description should always be set, if file was whiped for example they would dissapear bec its still a valid json file
		m_jsonObject["1_Header"] = headerName;
		m_jsonObject["2_Description"] = fileDescription;
		
		m_fileIsOpen = true;

	}

	void JsonSettings::SettingsFileClose(bool& errorCheck, const std::filesystem::path& filepath){

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

		m_jsonObject = nlohmann::json();
	}

	void JsonSettings::SettingsFileWhipe(bool& errorCheck){

		errorCheck = true;
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "cannot whipe file because it hasn't been opend yet";
			return;
		}

		m_jsonObject.clear();

	}

	void JsonSettings::SettingErase(bool& errorCheck, const std::string& name){

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

	void JsonSettings::SettingsFilePrettyPrintSet(const bool prettyPrint){
		m_prettyPrint = prettyPrint;
	}

	std::string JsonSettings::ErrorStringLastGet(){
		return m_lastErrorString;
	}


// Write methods
	void JsonSettings::SettingWriteBool(bool& errorCheck, const std::string& name, const bool value){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::SettingWriteInt(bool& errorCheck, const std::string& name, const int value) {
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::SettingWriteFloat(bool& errorCheck, const std::string& name, const float value) {
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}
	
	void JsonSettings::SettingWriteString(bool& errorCheck, const std::string& name, const std::string& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}


	void JsonSettings::SettingWriteVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::SettingWriteVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::SettingWriteVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

	void JsonSettings::SettingWriteVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& value) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you have to open the file first before you can write to it.";
		}

		m_jsonObject[name] = value;
	}

// Read Methods
	bool JsonSettings::SettingReadBool(bool& errorCheck, const std::string& name, const bool defaultValue){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}


		errorCheck = true;
		bool output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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

	int JsonSettings::SettingReadInt(bool& errorCheck, const std::string& name, const int defaultValue){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		int output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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
	

	float JsonSettings::SettingReadFloat(bool& errorCheck, const std::string& name, const float defaultValue){
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		float output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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

	std::string JsonSettings::SettingReadString(bool& errorCheck, const std::string& name, const std::string& defaultValue) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::string output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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

	std::vector<bool> JsonSettings::SettingReadVectorBool(bool& errorCheck, const std::string& name, const std::vector<bool>& defaultValue) {

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<bool> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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

	std::vector<int> JsonSettings::SettingReadVectorInt(bool& errorCheck, const std::string& name, const std::vector<int>& defaultValue) {
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<int> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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

	std::vector<float> JsonSettings::SettingReadVectorFloat(bool& errorCheck, const std::string& name, const std::vector<float>& defaultValue){
		
		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<float> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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

	std::vector<std::string> JsonSettings::SettingReadVectorString(bool& errorCheck, const std::string& name, const std::vector<std::string>& defaultValue){

		if(!m_fileIsOpen){
			errorCheck = false;
			m_lastErrorString = "you cannot read yet because no file has been opened yet";
		}

		errorCheck = true;
		std::vector<std::string> output = defaultValue;

		if(!m_jsonObject.contains(name)) {
			errorCheck = false;
			m_lastErrorString = "the given key does not exist yet - returning default";
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
