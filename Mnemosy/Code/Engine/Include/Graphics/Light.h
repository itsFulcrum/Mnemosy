#ifndef LIGHT_H
#define LIGHT_H

#include "Include/Graphics/Transform.h"

#include <glm/glm.hpp>


namespace mnemosy::graphics
{
	struct ModelData;
}

namespace mnemosy::graphics
{
	enum LightType
	{
		DIRECTIONAL = 0,
		POINT = 1
	};

	class Light
	{
	public:
		Light();
		~Light();

		void SetType(const LightType& lightType) { m_type = lightType; }

		const ModelData& GetModelData();

		const LightType& GetLightType();
		int GetLightTypeAsInt();

		Transform transform;
		glm::vec3 color = glm::vec3(1.0f,1.0f,1.0f);
		float strength = 1.0f;
		float falloff = 1.0f;
	private:
		LightType m_type = LightType::DIRECTIONAL;

		uint16_t m_directionalLightMesh_id;
		uint16_t m_pointLightMesh_id;

	};

} // mnemosy::graphics


#endif // !LIGHT_H
