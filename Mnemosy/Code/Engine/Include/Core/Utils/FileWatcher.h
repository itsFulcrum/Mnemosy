#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace mnemosy::core {

	class FileWatcher {

	public:
		FileWatcher();
		~FileWatcher();

		void RegisterFile(fs::path filePath);
		bool DidAnyFileChange();

	private:

		std::vector<fs::path> m_paths;
		std::vector<fs::file_time_type> m_fileTimeTypes;
	};

} // namespace mnemosy::core

#endif // !FILE_WATCHER_H
