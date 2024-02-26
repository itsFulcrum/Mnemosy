#include "Include/GuiPanels/SceneSettingsGuiPanel.h"

#include "Include/Application.h"

#include "Include/Graphics/Scene.h"
#include "Include/Graphics/Camera.h"
#include "Include/Graphics/Light.h"
#include "Include/Graphics/Skybox.h"
#include "Include/Graphics/RenderMesh.h"

#include "Include/Graphics/Renderer.h"
#include "External/ImGui/imgui.h"
#include "Include/MnemosyConfig.h"
#ifdef MNEMOSY_PLATFORM_WINDOWS
	#include "Include/Core/Utils/PlatfromUtils_Windows.h"
#endif // MNEMOSY_PLATFORM_WINDOWS

#include <glm/glm.hpp>
#include <iostream>

namespace mnemosy::gui
{
	SceneSettingsGuiPanel::SceneSettingsGuiPanel()
	{
		panelName = "Scene Settings";
	}

	void SceneSettingsGuiPanel::Draw()
	{
		if (!showPanel)
			return;

		graphics::Scene& scene = ENGINE_INSTANCE().GetScene();
		graphics::Renderer& renderer = ENGINE_INSTANCE().GetRenderer();

		ImGui::Begin(panelName.c_str(), &showPanel);
		
		// Render Mesh settings
		if (ImGui::TreeNode("Mesh Settings"))
		{
			graphics::RenderMesh& renderMesh = scene.GetMesh();
			
			//ImGui::Text("Mesh Settings");
			{
				const char* previewMesh_List[7] = { "Custom","Default","Cube","Plane","Sphere","Cylinder","Suzanne"}; // they need to be ordered the same as in lightType Enum in light class
				int previewMesh_Current = (int)scene.GetCurrentPreviewMesh();
				ImGui::Combo("Preview Mesh", &previewMesh_Current, previewMesh_List, IM_ARRAYSIZE(previewMesh_List));

				if ((int)scene.GetCurrentPreviewMesh() != previewMesh_Current)
				{
					if (previewMesh_Current == 0)
						scene.SetPreviewMesh(graphics::PreviewMesh::Custom);
					else if(previewMesh_Current == 1)
						scene.SetPreviewMesh(graphics::PreviewMesh::Default);
					else if(previewMesh_Current == 2)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cube);
					else if(previewMesh_Current == 3)
						scene.SetPreviewMesh(graphics::PreviewMesh::Plane);
					else if(previewMesh_Current == 4)
						scene.SetPreviewMesh(graphics::PreviewMesh::Sphere);
					else if(previewMesh_Current == 5)
						scene.SetPreviewMesh(graphics::PreviewMesh::Cylinder);
					else if(previewMesh_Current == 6)
						scene.SetPreviewMesh(graphics::PreviewMesh::Suzanne);
				}

				if (previewMesh_Current == 0)
				{
					if (ImGui::Button("Load Mesh..."))
					{

#ifdef MNEMOSY_PLATFORM_WINDOWS
						std::string filepath = mnemosy::core::FileDialogs::OpenFile("FBX (*.fbx)\0*.fbx\0 Obj (*.obj)\0*.obj\0");

						if (!filepath.empty())
						{
							renderMesh.LoadMesh(filepath.c_str());
						}
#endif
					}
				}

			}

			float meshPos[3] = { 1.0f,1.0f,1.0f };
			meshPos[0] = renderMesh.transform.GetPosition().x;
			meshPos[1] = renderMesh.transform.GetPosition().y;
			meshPos[2] = renderMesh.transform.GetPosition().z;
			ImGui::DragFloat3("Mesh Position", (float*)meshPos, 0.1f, -100, 100, "%0.1f");
			glm::vec3 newMeshPos = glm::vec3(meshPos[0], meshPos[1], meshPos[2]);
			renderMesh.transform.SetPosition(newMeshPos);

			float meshRot[3] = { 1.0f,1.0f,1.0f };
			meshRot[0] = renderMesh.transform.GetRotationEulerAngles().x;
			meshRot[1] = renderMesh.transform.GetRotationEulerAngles().y;
			meshRot[2] = renderMesh.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Mesh Rotation", (float*)meshRot, 0.1f, -360.0f, 360.0f, "%0.1f");
			glm::vec3 newMeshRot = glm::vec3(meshRot[0], meshRot[1], meshRot[2]);
			renderMesh.transform.SetRotationEulerAngles(newMeshRot);



			ImGui::TreePop();
		}

		// Skybox Settings
		if (ImGui::TreeNode("Skybox Settings"))
		{



			graphics::Skybox& skybox = scene.GetSkybox();

			if (ImGui::Button("Load File..."))
			{
				#ifdef MNEMOSY_PLATFORM_WINDOWS
					std::string filepath =  mnemosy::core::FileDialogs::OpenFile("All files (*.*)\0*.*\0 hdr (*.hdr)\0*.hdr\0 png (*.png)\0*.png\0 jpg (*.jpg)\0*.jpg\0");

					if (!filepath.empty())
					{
						skybox.AssignSkyboxTexture(filepath.c_str(),1024);
					}
				#endif
			}
			//ImGui::Text("Skybox Settings");
			ImGui::SliderFloat("Exposure", &skybox.exposure, -5.0f, 5.0f, "%.1f");
			ImGui::SliderFloat("Rotation", &skybox.rotation, 0.0f, 6.28f, "%1f");

			ImGui::ColorEdit3("Color Tint", (float*)&skybox.colorTint);
		
			renderer.SetShaderSkyboxUniforms();

			ImGui::TreePop();
		}

		// Light Settings
		if (ImGui::TreeNode("Light Settings"))
		{
			graphics::Light& light = scene.GetLight();

			//ImGui::Text("Light Settings");
			const char* lightTypes_List[] = { "Directional", "Point" }; // they need to be ordered the same as in lightType Enum in light class
			static int lightType_current = light.GetLightTypeAsInt();

			ImGui::Combo("Light Type", &lightType_current, lightTypes_List, IM_ARRAYSIZE(lightTypes_List));

			if (light.GetLightTypeAsInt() != lightType_current)
			{
				light.SetType((graphics::LightType)lightType_current);
			}

		
			float pos[3] = { 1.0f,1.0f,1.0f };
			pos[0] = light.transform.GetPosition().x;
			pos[1] = light.transform.GetPosition().y;
			pos[2] = light.transform.GetPosition().z;
			ImGui::DragFloat3("Position", (float*)pos, 0.1f, -100, 100, "%0.1f");
			glm::vec3 newPos = glm::vec3(pos[0], pos[1], pos[2]);
			light.transform.SetPosition(newPos);


			float rot[3] = { 1.0f,1.0f,1.0f };
			rot[0] = light.transform.GetRotationEulerAngles().x;
			rot[1] = light.transform.GetRotationEulerAngles().y;
			rot[2] = light.transform.GetRotationEulerAngles().z;
			ImGui::DragFloat3("Rotation", (float*)rot, 0.1f, -360.0f, 360.0f, "%0.1f");
			glm::vec3 newRot = glm::vec3(rot[0], rot[1], rot[2]);
			light.transform.SetRotationEulerAngles(newRot);

			ImGui::DragFloat("Strength",&light.strength,1.0f,0.0f,1000.0f,"%.1f");
			ImGui::SliderFloat("Falloff",&light.falloff,0.01f,5.0f,"%.1f");
			ImGui::ColorEdit3("Color", (float*)&light.color);

			renderer.SetPbrShaderLightUniforms();

			ImGui::TreePop();
		}

		// Camera Settings
		if (ImGui::TreeNode("Camera Settings"))
		{
			graphics::Camera& camera = scene.GetCamera();

			ImGui::SliderFloat("Field of View", &camera.settings.fov, 1.0f, 160.0f, "%.1f");

			ImGui::DragFloat("Near Clip", &camera.settings.nearClip, 0.1f, 0.00001f, 100.0f, "%0.4f");
			ImGui::DragFloat("Far Clip", &camera.settings.farClip, 0.1f, 0.02f, 5000.0f, "%0.4f");


			ImGui::TreePop();
		}

		// Post Processing Settings
		if (ImGui::TreeNode("Post processing"))
		{

			ImGui::Text("Here will go Post Processing settings");

			ImGui::TreePop();
		}

		//ImGui::ColorEdit3("MyColor##1", (float*)&color, misc_flags);

		ImGui::End();
	}

}