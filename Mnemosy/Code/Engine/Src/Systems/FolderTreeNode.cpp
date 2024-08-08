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


		for (size_t i = 0; i < subMaterials.size(); i++) {

			if (subMaterials[i]->name == name) {
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

		if (this->name == name)
			return true;

		if (!subNodes.empty()) {

			for (FolderNode* subnode : subNodes) {

				if (subnode->name == name) {
					return true;
				}
			}
		}

		return false;
	}


} // !mnemosy::systems