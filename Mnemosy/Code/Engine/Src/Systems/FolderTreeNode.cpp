#include "Include/Systems/FolderTreeNode.h"


namespace mnemosy::systems
{
	std::filesystem::path LibEntry::GetPathFromRoot()
	{
		namespace fs = std::filesystem;
		return parent->GetPathFromRoot() / fs::u8path(name);
	}



	// =======================  FolderNode

	bool FolderNode::IsLeafNode() {
		return subNodes.empty();
	}

	bool FolderNode::IsRoot() {

		if (parent == nullptr)
			return true;

		return false;
	}

	std::filesystem::path FolderNode::GetPathFromRoot() {


		namespace fs = std::filesystem;

		if (this->IsRoot()) {
			return fs::path();
		}

		fs::path rootPath = fs::u8path(this->name);

		FolderNode* currParent = this->parent;

		while (!currParent->IsRoot()) {

			rootPath = fs::u8path(currParent->name) / rootPath;

			currParent = currParent->parent;
		}

		return rootPath;
	}

	bool FolderNode::HasMaterials() {
		return !subEntries.empty();
	}

	void FolderNode::SortLibEntries()
	{
		// sort alphabetically
		if (!subEntries.empty()) {

			std::sort(subEntries.begin(), subEntries.end(), [](const LibEntry* a, const LibEntry* b) {
				return a->name < b->name;
			});
		}
	}





} // !mnemosy::systems