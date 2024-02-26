#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include "Include/Graphics/MeshData.h"
#include <vector>

namespace mnemosy::graphics 
{
	struct ModelData
	{
		std::vector<MeshData> meshes;
	};
} // mnemosy::graphics

#endif // !MODEL_DATA_H
