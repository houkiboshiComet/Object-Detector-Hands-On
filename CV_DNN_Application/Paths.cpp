#pragma unmanaged
#include "Paths.h"

#include <windows.h>
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")
#include<Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

namespace OpenCVApp {

	const std::string Paths::MODEL_DIR = "..\\model";
	const std::string Paths::ALEXNET_DIR = MODEL_DIR + "\\AlexNet";
	const std::string Paths::GOOGLENET_DIR = MODEL_DIR + "\\GoogLeNet";
	const std::string Paths::IMAGE_DIR = "..\\image";

	void Paths::createDirAsNecessary(const std::string& dir) {
		if (!PathFileExistsA(dir.c_str())) {
			MakeSureDirectoryPathExists(dir.c_str());
		}
	}
}

