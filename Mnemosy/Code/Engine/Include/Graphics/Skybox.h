#ifndef SKYBOX_H
#define SKYBOX_H

#include <glm/glm.hpp>
#include <string>

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

		bool AssignSkyboxTexture(const char* imagePath, const char* uniqueName, unsigned int resolution, bool savePermanently);
		void LoadPreviewSkybox(std::string name);

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

} // mnemosy::graphics

#endif // !SKYBOX_H
