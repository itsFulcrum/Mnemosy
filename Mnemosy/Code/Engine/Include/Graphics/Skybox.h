#ifndef SKYBOX_H
#define SKYBOX_H

#include <glm/glm.hpp>
#include <string>

namespace mnemosy::graphics
{
	enum CubemapType;
	class Cubemap;
	struct ModelData;
	class Texture;
}
namespace mnemosy::graphics
{
	class Skybox
	{
	public:
		Skybox() = default;
		~Skybox();

		void AssignCubemap(Cubemap* cubemap, CubemapType type);
		void RemoveAllCubemaps();

		bool IsColorCubeAssigned()		{ return m_colorCubemap; }
		bool IsIrradianceCubeAssigned() { return m_irradianceCubemap; }
		bool IsPrefilterCubeAssigned()	{ return m_prefilterCubemap; }

		Cubemap& GetColorCube()      { return *m_colorCubemap; }
		Cubemap& GetIrradianceCube() { return *m_irradianceCubemap; }
		Cubemap& GetPrefilterCube()  { return *m_prefilterCubemap; }

	public:
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		
		/*glm::vec3 sunDir = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float sunStrength = 1.0f;*/

		float exposure = 0.0f;

	private:
		Cubemap* m_colorCubemap = nullptr;
		Cubemap* m_irradianceCubemap = nullptr;
		Cubemap* m_prefilterCubemap = nullptr;

	};

} // mnemosy::graphics

#endif // !SKYBOX_H
