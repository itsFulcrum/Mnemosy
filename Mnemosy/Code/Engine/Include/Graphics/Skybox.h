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
		void LoadPreviewSkybox(const std::string& name);

		Cubemap& GetCubemap();
		ModelData& GetModelData();

	public:
		glm::vec3 colorTint = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 backgroundColor = glm::vec3(0.2f, 0.2f, 0.2f);

		float rotation = 0.0f;
		float exposure = 0.0f;
		float opacity = 0.0f;
		float gradientOpacity = 1.0f;
		float blurRadius = 0.0f;
		int blurSteps = 0;

	private:
		uint16_t m_modelData_id;


		Cubemap* m_pCubemap = nullptr;

	};

} // mnemosy::graphics

#endif // !SKYBOX_H
