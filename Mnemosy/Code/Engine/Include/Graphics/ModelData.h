#ifndef GRAPHICS_MODEL_DATA_H
#define GRAPHICS_MODEL_DATA_H

#include "Engine/Include/Graphics/MeshData.h"
#include <vector>


namespace mnemosy::graphics 
{
	struct ModelData
	{
		std::vector<MeshData> meshes;
	};
}

#endif // !GRAPHICS_MODEL_DATA_H
