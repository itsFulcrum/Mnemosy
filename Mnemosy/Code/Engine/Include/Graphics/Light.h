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

		void SetType(LightType lightType) { m_type = lightType; }

		ModelData& GetModelData();

		LightType GetLightType();
		int GetLightTypeAsInt();

		Transform transform;
		glm::vec3 color = glm::vec3(1.0f,1.0f,1.0f);
		float strength = 1.0f;
		float falloff = 1.0f;
	private:
		LightType m_type = LightType::DIRECTIONAL;

		ModelData* m_pDirectionalLightMesh;
		ModelData* m_pPointLightMesh;


	};

} // mnemosy::graphics


#endif // !LIGHT_H
