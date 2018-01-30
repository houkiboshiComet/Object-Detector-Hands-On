#pragma unmanaged
#include "UserCommunicator.h"

#include <stdlib.h>
#include <windows.h>
#include <math.h>

namespace OpenCVApp {
	
	std::string UserCommunicator::askForFilename() {
		OPENFILENAMEA ofn;
		char nameBuff[MAX_PATH] = "";
		
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = "“®‰æƒtƒ@ƒCƒ‹(*.avi,*.mp4)\0*.avi;*.mp4\0";
		ofn.lpstrFile = nameBuff;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn)) {
			return std::string(nameBuff);
		}
		else {
			return "";
		}
	}

	
}