#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#include "Include/Graphics/Transform.h"

namespace mnemosy::graphics
{
	//struct Transform; // idk not working
	class Material;
	struct ModelData;
}

namespace mnemosy::graphics
{

	class RenderMesh
	{
	public:
		RenderMesh();
		RenderMesh(const char* path);
		~RenderMesh();

		Material& GetMaterial() { return *m_material; }
		void SetMaterial(Material* material);

		void LoadMesh(const char* path);

		ModelData& GetModelData();


		Transform transform;
	
	private:
		void ClearModelData();


		Material* m_material = nullptr;
		ModelData* m_modelData = nullptr;

	};

} // mnemosy::graphics

#endif // !RENDER_MESH_H
