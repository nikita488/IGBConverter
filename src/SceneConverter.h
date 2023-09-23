#pragma once

#include <fbxsdk.h>
#include <igCore/igStringObj.h>
#include <igAttrs/igMaterialAttr.h>
#include <igAttrs/igTextureBindAttr.h>
#include <igAttrs/igTextureAttr.h>
#include <igGfx/igImage.h>
#include <igSg/igIterateGraph.h>
#include <igSg/igNode.h>
#include <igSg/igGroup.h>
#include <igSg/igGeometry.h>
#include <igSg/igTransform.h>
#include <igSg/igCamera.h>
#include <igSg/igLightSet.h>
#include "GeometryWrapper.h"
#include "Utils.h"

using namespace Gap;
using namespace Sg;
using namespace fbxsdk;

namespace Converter
{
	class SceneConverter
	{
	private:
		FbxScene* scene;
		igNodeRef sceneGraph;
		igStringObjRef stringHelper;
		igMaterialAttr* lastMaterialAttr = NULL;
		igTextureBindAttr* lastTextureBindAttr = NULL;
		FbxSurfaceMaterial* lastMaterial = NULL;
		FbxTexture* lastTexture = NULL;

	public:
		SceneConverter(FbxScene* scene, igNode* sceneGraph);
		~SceneConverter();

		void Convert();
	private:
		FbxNode* ProcessNode(igNode* node);
		void ProcessTransform(igTransform* transform, FbxNode* node);
		void ProcessGeometry(igGeometry* geometry, FbxNode* node);
		void ProcessLight(igLightSet* lightSet, FbxNode* node);
		void ProcessCamera(igCamera* camera, FbxNode* node);
	};
}