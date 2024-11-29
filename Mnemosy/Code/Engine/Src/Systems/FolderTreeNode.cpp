#include "Include/Systems/FolderTreeNode.h"


namespace mnemosy::systems
{

	bool FolderNode::IsLeafNode() {
		return subNodes.empty();
	}

	bool FolderNode::IsRoot() {

		if (parent == nullptr)
			return true;

		return false;
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
	std::filesystem::path LibEntry::GetPathFromRoot()
	{
		return parent->pathFromRoot / std::filesystem::path(name);
	}

} // !mnemosy::systems