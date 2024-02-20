#ifndef GRAPHICS_SKYBOX_H
#define GRAPHICS_SKYBOX_H



#include <glm/glm.hpp>

namespace mnemosy::graphics
{
	class Cubemap;
	struct ModelData;
}
namespace mnemosy::graphics
{
	class Skybox
	{
	public:
		Skybox();
		Skybox(const char* imagePath, unsigned int resolution);

		~Skybox();

		void AssignSkyboxTexture(const char* imagePath, unsigned int resolution);


		Cubemap& GetCubemap();
		ModelData& GetModelData();

	public:
		float rotation = 0;
		float exposure = 0;
		glm::vec3 colorTint = glm::vec3(1.0, 1.0f, 1.0f);

	private:
		ModelData* m_pModelData = nullptr;
		Cubemap* m_pCubemap = nullptr;

	};


}

#endif // !GRAPHICS_SKYBOX_H
