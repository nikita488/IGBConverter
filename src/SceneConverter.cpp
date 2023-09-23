#include "SceneConverter.h"

namespace Converter
{
	SceneConverter::SceneConverter(FbxScene* scene, igNode* sceneGraph) :
		scene(scene),
		sceneGraph(sceneGraph),
		stringHelper(igStringObj::instantiateRefFromPool(kIGMemoryPoolTemporary))
	{}

	SceneConverter::~SceneConverter()
	{
		sceneGraph = NULL;
		stringHelper = NULL;
		lastMaterialAttr = NULL;
		lastTextureBindAttr = NULL;
		lastMaterial = NULL;
		lastTexture = NULL;
	}

	void SceneConverter::Convert()
	{
		scene->GetRootNode()->AddChild(ProcessNode(sceneGraph));
	}

	FbxNode* SceneConverter::ProcessNode(igNode* node)
	{
		FbxNode* fbxNode = FbxNode::Create(scene, node->getName());

		if (node->isOfType(igTransform::getClassMeta()))
			ProcessTransform(static_cast<igTransform*>(node), fbxNode);
		else if (node->isOfType(igGeometry::getClassMeta()))
			ProcessGeometry(static_cast<igGeometry*>(node), fbxNode);
		else if (node->isOfType(igLightSet::getClassMeta()))
			ProcessLight(static_cast<igLightSet*>(node), fbxNode);
		else if (node->isOfType(igCamera::getClassMeta()))
			ProcessCamera(static_cast<igCamera*>(node), fbxNode);

		if (!node->hasChildren())
			return fbxNode;

		igGroup* group = igGroup::dynamicCast(node);

		if (group)
			for (igUnsignedInt i = 0; i < group->getChildCount(); i++)
				fbxNode->AddChild(ProcessNode(group->getChild(i)));
		return fbxNode;
	}

	void SceneConverter::ProcessTransform(igTransform* transform, FbxNode* node)
	{
		FbxMatrix nodeTM = Utils::ig2FbxMatrix(*transform->getMatrix());
		FbxVector4 translation;
		FbxVector4 rotation;
		FbxVector4 scaling;
		FbxVector4 shearing;
		double sign;

		nodeTM.GetElements(translation, rotation, shearing, scaling, sign);
		node->LclTranslation.Set(translation);
		node->LclRotation.Set(rotation);
		node->LclScaling.Set(scaling);
	}

	void SceneConverter::ProcessGeometry(igGeometry* geometry, FbxNode* node)
	{
		GeometryWrapper wrapper(geometry);

		if (!wrapper.configured())
			return;

		igVertexFormat format = wrapper.getVertexFormat();
		FbxMesh* mesh = FbxMesh::Create(scene, geometry->getName());
		igMaterialAttr* materialAttr = Utils::findParentAttribute<igMaterialAttr>(geometry);
		igTextureBindAttr* textureBindAttr = Utils::findParentAttribute<igTextureBindAttr>(geometry, 0);

		if (materialAttr != lastMaterialAttr || textureBindAttr != lastTextureBindAttr)
		{
			FbxSurfacePhong* material = FbxSurfacePhong::Create(scene, geometry->getName());
			
			material->ShadingModel = "Phong";

			if (materialAttr)
			{
				igVec4f diffuse = *materialAttr->getDiffuse();

				material->Diffuse = Utils::ig2FbxDouble3(diffuse);
				material->Ambient = Utils::ig2FbxDouble3(*materialAttr->getAmbient());
				material->Specular = Utils::ig2FbxDouble3(*materialAttr->getSpecular());
				material->Shininess = materialAttr->getShininess() / 128.0F;
				material->TransparencyFactor = 1.0F - diffuse[3];
			}
			else
			{
				FbxDouble3 white = FbxDouble3(1.0, 1.0, 1.0);
				FbxDouble3 black = FbxDouble3(0.0, 0.0, 0.0);

				material->Diffuse.Set(white);
				material->Ambient.Set(white);
				material->Specular.Set(black);
				material->Shininess = 0.0F;
			}
			
			if (textureBindAttr && textureBindAttr->getTexture())
			{
				if (textureBindAttr != lastTextureBindAttr)
				{
					igTextureAttr* textureAttr = textureBindAttr->getTexture();

					if (textureAttr && textureAttr->getImageCount() > 0)
					{
						igImage* image = textureAttr->getImage();

						stringHelper->set(image->getName());
						stringHelper->removePathFromFileName();
						stringHelper->removeFileExtension();

						FbxFileTexture* texture = FbxFileTexture::Create(scene, stringHelper->getBuffer());

						stringHelper->set(image->getName());
						stringHelper->removePathFromFileName();
						texture->SetFileName(stringHelper->getBuffer());
						texture->SetTextureUse(FbxTexture::eStandard);
						texture->SetMappingType(FbxTexture::eUV);
						texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
						//texture->SetSwapUV(false);
						//texture->SetTranslation(0.0, 0.0);
						//texture->SetScale(1.0, 1.0);
						//texture->SetRotation(0.0, 0.0);
						material->Diffuse.ConnectSrcObject(texture);
						lastTextureBindAttr = textureBindAttr;
						lastTexture = texture;
					}
				}
				else
				{
					material->Diffuse.ConnectSrcObject(lastTexture);
				}
			}

			node->AddMaterial(material);
			lastMaterial = material;
			lastMaterialAttr = materialAttr;
		}
		else
		{
			node->AddMaterial(lastMaterial);
		}

		node->SetNodeAttribute(mesh);
		mesh->InitControlPoints(wrapper.getVertexCount());

		if (node->GetMaterialCount() > 0)
		{
			FbxGeometryElementMaterial* material = mesh->CreateElementMaterial();

			material->SetMappingMode(FbxGeometryElement::eAllSame);
			material->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
			material->GetIndexArray().Add(0);
		}

		if (format.hasNormals())
		{
			FbxGeometryElementNormal* normal = mesh->CreateElementNormal();

			normal->SetMappingMode(FbxGeometryElement::eByControlPoint);
			normal->SetReferenceMode(FbxGeometryElement::eDirect);
			normal->GetDirectArray().SetCount(mesh->GetControlPointsCount());
		}

		if (format.hasVertexColors())
		{
			FbxGeometryElementVertexColor* color = mesh->CreateElementVertexColor();

			color->SetMappingMode(FbxGeometryElement::eByControlPoint);
			color->SetReferenceMode(FbxGeometryElement::eDirect);
			color->GetDirectArray().SetCount(mesh->GetControlPointsCount());
		}

		if (format.getTextureCoordCount() > 0)
		{
			FbxGeometryElementUV* uv = mesh->CreateElementUV("Diffuse");

			uv->SetMappingMode(FbxGeometryElement::eByControlPoint);
			uv->SetReferenceMode(FbxGeometryElement::eDirect);
			uv->GetDirectArray().SetCount(mesh->GetControlPointsCount());
		}

		igUnsignedInt uvIndex = 0;
		FbxLayerElement::EType uvType = FbxLayerElement::eTextureDiffuse;
		FbxVector4* controlPoints = mesh->GetControlPoints();

		for (igInt i = 0; i < mesh->GetControlPointsCount(); i++)
		{
			controlPoints[i] = Utils::ig2FbxVector4(wrapper.getPosition(i));

			if (mesh->GetElementNormalCount() > 0)
				mesh->GetElementNormal()->GetDirectArray().SetAt(i, Utils::ig2FbxVector4(wrapper.getNormal(i)));

			if (mesh->GetElementVertexColorCount() > 0)
				mesh->GetElementVertexColor()->GetDirectArray().SetAt(i, Utils::ig2FbxColor(wrapper.getColor(i)));

			if (mesh->GetElementUVCount(uvType) > 0)
				mesh->GetElementUV(uvIndex, uvType)->GetDirectArray().SetAt(i, Utils::ig2FbxVector2(wrapper.getTextureCoord(uvIndex, i)));
		}

		igIndexArray* indices = wrapper.getIndexArray();

		for (igUnsignedInt i = 0; i < wrapper.getPrimitiveCount(); i++)
		{
			mesh->BeginPolygon();
			mesh->AddPolygon(indices->getIndex(i * 3 + 0));
			mesh->AddPolygon(indices->getIndex(i * 3 + 1));
			mesh->AddPolygon(indices->getIndex(i * 3 + 2));
			mesh->EndPolygon();
		}
	}

	void SceneConverter::ProcessLight(igLightSet* lightSet, FbxNode* node)
	{
		for (igUnsignedInt i = 0; i < lightSet->getLightCount(); i++)
		{
			igLightAttr* lightAttr = lightSet->getLight(i);
			igVec3f pos = *lightAttr->getPosition();
			igVec3f dir = *lightAttr->getDirection();
			igVec3f attenuation = *lightAttr->getAttenuation();
			igVec4f diffuse = *lightAttr->getDiffuse();

			if (strcmp(lightSet->getName(), "SceneAmbient") == 0)
			{
				igVec4f ambient = *lightAttr->getAmbient();
				scene->GetGlobalSettings().SetAmbientColor(Utils::ig2FbxColor(ambient));
				return;
			}

			FbxLight* light = FbxLight::Create(scene, lightSet->getName());

			node->SetNodeAttribute(light);
			node->LclTranslation = Utils::ig2FbxDouble3(pos);
			//node->LclRotation = FbxDouble3(dir[0], dir[1], dir[2]);

			//::FbxVector4 direction(dir[0], dir[1], dir[2]);
			//::FbxVector4 upVector(0, 0, 1);
			//fbxsdk::FbxVector4 rotationAxis = upVector.CrossProduct(direction);
			//double dotProduct = upVector.DotProduct(direction);
			//double rotationAngle = acos(dotProduct) * FBXSDK_RAD_TO_DEG;
			//::FbxQuaternion rotationQuat(rotationAxis, rotationAngle);
			//fbxsdk::FbxVector4 d = rotationQuat.DecomposeSphericalXYZ();

			//node->LclRotation.Set(d);
			//FbxDouble3 r = node->LclRotation.Get();

			//printf("Rotation %s %f [%f,%f,%f]\n", lightSet->getName(), rotationAngle, r[0], r[1], r[2]);

			switch (lightAttr->getType())
			{
			case Gap::Gfx::IG_GFX_LIGHT_TYPE_DIRECTIONAL:
				light->LightType = FbxLight::EType::eDirectional;
				break;
			case Gap::Gfx::IG_GFX_LIGHT_TYPE_POINT:
				light->LightType = FbxLight::EType::ePoint;
				break;
			case Gap::Gfx::IG_GFX_LIGHT_TYPE_SPOT:
				light->LightType = FbxLight::EType::eSpot;
				break;
			}

			light->Color = Utils::ig2FbxDouble3(diffuse);

			if (attenuation[0] > 0)
			{
				light->DecayType = FbxLight::EDecayType::eNone;
				light->Intensity = (1 / attenuation[0]) * 100;
			}
			else if (attenuation[1] > 0)
			{
				light->DecayType = FbxLight::EDecayType::eLinear;
				light->Intensity = (0.025F / attenuation[1]) * 100;
			}
			else if (attenuation[2] > 0)
			{
				light->DecayType = FbxLight::EDecayType::eQuadratic;
				light->Intensity = (0.0006F / attenuation[2]) * 100;
			}

			light->DecayStart = light->Intensity / 200;

			light->OuterAngle = lightAttr->getCutoffAngle() * 2;
			light->InnerAngle = (1 - lightAttr->getFalloffRate() * 30) * light->OuterAngle;
		}
	}

	void SceneConverter::ProcessCamera(igCamera* camera, FbxNode* node)
	{
		FbxCamera* fbxCamera = FbxCamera::Create(scene, camera->getName());

		fbxCamera->NearPlane = camera->getNearPlane();
		fbxCamera->FarPlane = camera->getFarPlane();
		fbxCamera->FieldOfView = camera->getHorizontalFieldOfView();

		node->SetNodeAttribute(fbxCamera);
	}
}