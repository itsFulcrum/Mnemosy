#include "Include/Core/Utils/FileWatcher.h"

#include "Include/Core/Log.h"

namespace mnemosy::core {

	FileWatcher::FileWatcher() {

	}
	FileWatcher::~FileWatcher() {

		if (!m_paths.empty()) {
			m_paths.clear();
		}

		if (!m_fileTimeTypes.empty()) {
			m_fileTimeTypes.clear();
		}

	}

	void FileWatcher::RegisterFile(fs::path filePath) {


		fs::directory_entry file = fs::directory_entry(filePath);

		if (!file.exists()) {
			MNEMOSY_ERROR("FileWatcher::RegisterFile: Filepath does not exist. {}", filePath.generic_string());
			return;
		}

		if (!file.is_regular_file()) {
			MNEMOSY_ERROR("FileWatcher::RegisterFile: Filepath is not a file. {}", filePath.generic_string());
			return;
		}

		//MNEMOSY_DEBUG("FileWatcher::RegisterFile: {}", filePath.generic_string());



		m_paths.push_back(filePath);
		m_fileTimeTypes.push_back(file.last_write_time());
	}

	bool FileWatcher::DidAnyFileChange() {

		if (m_paths.empty()) {
			return false;
		}


		bool changed = false;
		for (size_t i = 0; i < m_paths.size(); i++) {


			fs::directory_entry dir = fs::directory_entry(m_paths[i]);

			if (m_fileTimeTypes[i] != dir.last_write_time()) {

				m_fileTimeTypes[i] = dir.last_write_time();
				changed = true;
				return true;
				// we want to continue the loop here to update all remaining files in the list.
			}
		}

		return changed;
	}


} // namespace mnemosy::core