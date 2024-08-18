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


	bool FolderNode::SubMaterialExistsAlready(std::string& name) {

		if (subMaterials.empty())
			return false;


		std::string nameLower = MakeStringLowerCase(name);


		for (size_t i = 0; i < subMaterials.size(); i++) {

			if (MakeStringLowerCase(subMaterials[i]->name) == nameLower) {
				return true;
			}
		}

		return false;
	}

	bool FolderNode::HasMaterials() {
		return !subMaterials.empty();
	}

	FolderNode* FolderNode::GetSubNodeByName(std::string& name) {
		
		if (!subNodes.empty()) {

			for (FolderNode* node : subNodes) {

				if (node->name == name) {
					return node;
				}
			}
		}

		return nullptr;
	}

	bool FolderNode::SubnodeExistsAlready(std::string& name) {

		// only searches curent and subnodes not reqursivly the entire subtree

		std::string nameLower = MakeStringLowerCase(name);


		if (MakeStringLowerCase(this->name) == nameLower) {
			return true;
		}

		if (!subNodes.empty()) {

			for (FolderNode* subnode : subNodes) {

				if (MakeStringLowerCase(subnode->name) == nameLower) {
					return true;
				}
			}
		}

		return false;
	}


} // !mnemosy::systems