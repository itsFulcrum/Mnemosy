#include "Include/Graphics/Skybox.h"

#include "Include/MnemosyEngine.h"
#include "Include/Systems/SkyboxAssetRegistry.h"
#include "Include/Core/FileDirectories.h"


#include "Include/Graphics/Cubemap.h"
#include "Include/Graphics/Texture.h"
#include "Include/Graphics/ModelData.h"

#include "Include/Core/Log.h"
#include "Include/Graphics/ModelLoader.h"
#include "Include/Systems/MeshRegistry.h"

#include <filesystem>

namespace mnemosy::graphics
{
	Skybox::~Skybox() {
		RemoveAllCubemaps();
	}

	void Skybox::AssignCubemap(Cubemap* cubemap, CubemapType type)
	{
		switch (type)
		{
		//case mnemosy::graphics::MNSY_CUBEMAP_TYPE_COLOR:

		//	if (m_colorCubemap) {
		//		delete m_colorCubemap;

		//	}
		//	m_colorCubemap = cubemap;

		//	break;
		case mnemosy::graphics::MNSY_CUBEMAP_TYPE_IRRADIANCE:

			if (m_irradianceCubemap) {

				delete m_irradianceCubemap;
			}

			m_irradianceCubemap = cubemap;

			break;
		case mnemosy::graphics::MNSY_CUBEMAP_TYPE_PREFILTER:
			
			if (m_prefilterCubemap) {
				delete m_prefilterCubemap;
			}
			m_prefilterCubemap = cubemap;

			break;
		case mnemosy::graphics::MNSY_CUBEMAP_TYPE_COUNT:
			break;

		case mnemosy::graphics::MNSY_CUBEMAP_TYPE_NONE:
			break;
		default:
			break;
		}
	}

	void Skybox::RemoveAllCubemaps()
	{

		/*if (m_colorCubemap) {
			delete m_colorCubemap;
			m_colorCubemap = nullptr;
		}*/

		if (m_irradianceCubemap) {
			delete m_irradianceCubemap;
			m_irradianceCubemap = nullptr;
		}

		if (m_prefilterCubemap) {
			delete m_prefilterCubemap;
			m_prefilterCubemap = nullptr;
		}
	}

}