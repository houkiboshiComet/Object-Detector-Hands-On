#pragma once
#include <string>

namespace OpenCVApp {

	class  Paths
	{
	public:
		static const std::string MODEL_DIR;
		static const std::string ALEXNET_DIR;
		static const std::string GOOGLENET_DIR;
		static const std::string IMAGE_DIR;

		static void createDirAsNecessary(const std::string& dir);
	};
}