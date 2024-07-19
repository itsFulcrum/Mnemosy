#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include "Include/Graphics/Transform.h"

namespace mnemosy::graphics
{
	struct ModelData;
}

namespace mnemosy::graphics
{

	class RenderMesh {
	public:
		RenderMesh();
		RenderMesh(const char* path);
		~RenderMesh();

		void LoadMesh(const char* path);
		ModelData& GetModelData();

		Transform transform;
	
	private:
		void ClearModelData();
		
		//ModelData* m_modelData = nullptr;
		unsigned int m_modelData_id;

	};

} // mnemosy::graphics

#endif // !RENDER_MESH_H
