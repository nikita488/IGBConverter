#pragma once

#include <fbxsdk.h>
#include <igMath/igVec3f.h>

using namespace fbxsdk;

namespace Converter
{
	class ConverterManager
	{
	private:
		FbxManager* manager;
		FbxExporter* exporter;
		FbxScene* scene;

	public:
		ConverterManager(Gap::Math::igVec3f upVector);
		~ConverterManager();

		bool Initialize(const char* fileName);
		bool Export();

		FbxScene* GetScene();
	};
}