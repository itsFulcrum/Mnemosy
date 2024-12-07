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

		// TODO: implement
		bool IsTextureAssigned() { return false; }
		void RemoveCubemap();


		Cubemap& GetCubemap();
		ModelData& GetModelData();

	public:
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 sunDir = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float sunStrength = 1.0f;
		float exposure = 0.0f;

	private:
		// there is no need to have this  part of this class
		uint16_t m_modelData_id; 


		Cubemap* m_pCubemap = nullptr;

	};

} // mnemosy::graphics

#endif // !SKYBOX_H
