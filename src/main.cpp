#include "main.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage : %s <inputFile.igb> <outputFile.fbx|dae|obj>\n", argv[0]);
		return 1;
	}

	igAlchemy alchemy;
	Sg::igArkRegisterForIGBFiles();
	ArkCore->getPluginHelper()->loadPlugin("libRaven", "Alchemy");

	igString inputFile = argv[1];
	igString outputFile = argv[2];

	if (!igIGBResource->load(inputFile))
	{
		fprintf(stderr, "Failed to load IGB file %s\n", inputFile);
		return -1;
	}

	Sg::igSceneInfoRef sceneInfo = Sg::igSceneInfo::dynamicCast(igIGBResource->getInfoByType(inputFile, "igSceneInfo"));

	igIGBResource->unload(inputFile);

	if (!sceneInfo)
	{
		fprintf(stderr, "Failed to get igSceneInfo from IGB file %s\n", inputFile);
		return -2;
	}
	
	Converter::ConverterManager manager(sceneInfo->_upVector);

	if (!manager.Initialize(outputFile))
	{
		fprintf(stderr, "Failed to initialize exporter for %s\n", outputFile);
		return -3;
	}

	Converter::SceneConverter converter(manager.GetScene(), sceneInfo->getSceneGraph());

	sceneInfo = NULL;
	converter.Convert();

	if (!manager.Export())
	{
		fprintf(stderr, "Failed to export %s\n", outputFile);
		return -4;
	}

	return 0;
}