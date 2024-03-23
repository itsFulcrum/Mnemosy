#include "Include/Systems/ThumbnailManager.h"

#include "Include/Core/Log.h"
#include "Include/MnemosyEngine.h"
#include "Include/Graphics/Renderer.h"
#include "Include/Core/FileDirectories.h"
#include "Include/Graphics/Scene.h"
#include "Include/Systems/FolderTreeNode.h"
#include "Include/Graphics/Utils/KtxImage.h"
#include "Include/Graphics/Material.h"
#include "Include/Graphics/ThumbnailScene.h"
#include "Include/Graphics/Camera.h"

#include <glad/glad.h>



namespace mnemosy::systems {

	ThumbnailManager::ThumbnailManager() {
	
		CreateThumbnailFramebuffers();
	}

	ThumbnailManager::~ThumbnailManager() {
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteRenderbuffers(1, &m_rbo);
		glDeleteTextures(1, &m_renderTexture);
		glDeleteFramebuffers(1, &m_blitFbo);
		glDeleteTextures(1, &m_thumbnailRenderTexture_Id);
	}

	void ThumbnailManager::RenderThumbnailOfActiveMaterial(fs::path& pathToThumbnail, FolderNode* selectedFolder,unsigned int activeMaterialID) {


		graphics::Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		fs::path thumbnailPath = fs::path(activeMat.Name + "_thumbnail.ktx2");
		fs::path thumbnailAbsolutePath = pathToThumbnail;

		graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();
		renderer.RenderThumbnail(activeMat);

		graphics::KtxImage thumbnailKtx;
		thumbnailKtx.ExportGlTexture(thumbnailAbsolutePath.generic_string().c_str(), renderer.GetThumbnailRenderTextureID(), 3, renderer.GetThumbnailResolution(), renderer.GetThumbnailResolution(), graphics::ktxImgFormat::MNSY_COLOR, false);

		// check if the active material is part of the currently opend folder and refresh if needed
		if (selectedFolder->HasMaterials()) {
			for (systems::MaterialInfo mat : selectedFolder->subMaterials) {

				if (mat.runtime_ID == activeMaterialID) {

					RefreshThumbnail(mat, thumbnailAbsolutePath);
					break;
				}
			}
		}



	}
	void ThumbnailManager::RefreshThumbnail(MaterialInfo& materialInfo,fs::path& pathToThumbnail) {

		glDeleteTextures(1, &materialInfo.thumbnailTexure_ID);
		materialInfo.thumbnailTexure_ID = 0;
		glGenTextures(1, &materialInfo.thumbnailTexure_ID);

		graphics::KtxImage thumbnailImg;
		thumbnailImg.LoadKtx(pathToThumbnail.generic_string().c_str(), materialInfo.thumbnailTexure_ID);
	}
	void ThumbnailManager::DeleteThumbnailFromCache(MaterialInfo& materialInfo) {
		glDeleteTextures(1, &materialInfo.thumbnailTexure_ID);
		materialInfo.thumbnailTexure_ID = 0;
		materialInfo.thumbnailLoaded = false;
		MNEMOSY_TRACE("Deleted thumbnail: {}, from memory", materialInfo.name);

	}


	void ThumbnailManager::DeleteLoadedThumbnailsOfActiveFolder(FolderNode* activeFolder)  {

		if (activeFolder == nullptr)
			return;

		// delete currently loaded thumbnails
		if (activeFolder->HasMaterials()) {

			for (int i = 0; i < activeFolder->subMaterials.size(); i++) {

				if (activeFolder->subMaterials[i].thumbnailLoaded) {
					glDeleteTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);
					activeFolder->subMaterials[i].thumbnailLoaded = false;
					activeFolder->subMaterials[i].thumbnailTexure_ID = 0;
					MNEMOSY_TRACE("Deleted thumbnail for {}", activeFolder->subMaterials[i].name);
				}
			}
		}

		m_activeFolderFullyLoaded = false;

	}

	void ThumbnailManager::LoadThumbnailsOfActiveFolder(FolderNode* activeFolder, fs::path folderDirectory) {

		if (activeFolder == nullptr)
			return;

		if (m_activeFolderFullyLoaded)
			return;

		for (int i = 0; i < activeFolder->subMaterials.size(); i++) {

			if (!activeFolder->subMaterials[i].thumbnailLoaded) {

				glGenTextures(1, &activeFolder->subMaterials[i].thumbnailTexure_ID);

				fs::path thumbnailPath = folderDirectory / fs::path(activeFolder->subMaterials[i].name) / fs::path(activeFolder->subMaterials[i].name + "_thumbnail.ktx2");

				graphics::KtxImage ktxThumbnail;
				bool success = ktxThumbnail.LoadKtx(thumbnailPath.generic_string().c_str(), activeFolder->subMaterials[i].thumbnailTexure_ID);
				if (!success) {

					MNEMOSY_TRACE("Loaded thumbnail FAILED: {}", activeFolder->subMaterials[i].name);
				}

				activeFolder->subMaterials[i].thumbnailLoaded = true;
				MNEMOSY_TRACE("Loaded thumbnail for {}", activeFolder->subMaterials[i].name);


				if(i == activeFolder->subMaterials.size()){
					m_activeFolderFullyLoaded = true;
					MNEMOSY_TRACE("Thumbnails fully loaded");
				}

				// When calling this functions each frame we should break here to only load one thumbnail at a time
				break;
			}
		}
	}

	void ThumbnailManager::CreateThumbnailFramebuffers()
	{
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		glGenTextures(1, &m_renderTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_renderTexture);

		// rendering thumbnails with msaa4 always
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, m_thumbnailResolution, m_thumbnailResolution, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,m_renderTexture, 0);
		

		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, m_thumbnailResolution, m_thumbnailResolution);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

		MNEMOSY_ASSERT(glad_glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Faild to complete framebuffer");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// for blitting thumbnail texture

		glGenFramebuffers(1, &m_blitFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_blitFbo);


		glGenTextures(1, &m_thumbnailRenderTexture_Id);
		glBindTexture(GL_TEXTURE_2D, m_thumbnailRenderTexture_Id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_thumbnailResolution, m_thumbnailResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_thumbnailRenderTexture_Id, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);


		MNEMOSY_DEBUG("Renderer: Framebuffer created");


	}

	void ThumbnailManager::RenderThumbnail(graphics::Material& activeMaterial) {

		////Material& activeMat = MnemosyEngine::GetInstance().GetScene().GetActiveMaterial();

		//graphics::ThumbnailScene& thumbScene = MnemosyEngine::GetInstance().GetThumbnailScene();
		//graphics::Renderer& renderer = MnemosyEngine::GetInstance().GetRenderer();


		//unsigned int width = m_thumbnailResolution;
		//unsigned int height = m_thumbnailResolution;

		//renderer.SetPbrShaderLightUniforms(thumbScene.GetLight());
		//renderer.SetShaderSkyboxUniforms(thumbScene.GetSkybox());

		//thumbScene.GetCamera().SetScreenSize(width, height);

		//m_projectionMatrix = thumbScene.GetCamera().GetProjectionMatrix();
		//m_viewMatrix = thumbScene.GetCamera().GetViewMatrix();


		////m_camera->SetScreenSize(instance.GetWindow().GetViewportWidth(), instance.GetWindow().GetViewportHeight());
		////m_scene->GetCamera().SetScreenSize(m_pWindow->GetWindowWidth(), m_pWindow->GetWindowHeight());

		////instance.GetRenderer().SetViewMatrix(m_camera->GetViewMatrix());
		////instance.GetRenderer().SetProjectionMatrix(m_camera->GetProjectionMatrix());


		//StartFrame(width, height);


		////SetPbrShaderGlobalSceneUniforms(scene.GetSkybox(), scene.GetLight(), scene.GetCamera().transform.GetPosition());

		//m_pPbrShader->Use();


		//glm::vec3 cameraPosition = thumbScene.GetCamera().transform.GetPosition();
		//m_pPbrShader->SetUniformFloat3("_cameraPositionWS", cameraPosition.x, cameraPosition.y, cameraPosition.z);

		//activeMaterial.setMaterialUniforms(*m_pPbrShader);

		//RenderMeshes(thumbScene.GetMesh());


		////RenderGizmo(scene.GetGizmoMesh());
		//RenderLightMesh(thumbScene.GetLight());

		////RenderSkybox(thumbScene.GetSkybox());

		////EndFrame
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_blitFbo);
		//glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);




		//// restore user settings


		//graphics::Scene& scene = MnemosyEngine::GetInstance().GetScene();
		//renderer.SetPbrShaderLightUniforms(scene.GetLight());
		//renderer.SetShaderSkyboxUniforms(scene.GetSkybox());


	}




} // ! mnemosy::systems