#include "utils/FileSystemUtils.hpp"
#include <sys/stat.h>
#include <sys/types.h>

namespace FileSystemUtils
{
	void ensureOutputDirectoryExists()
	{
		const char* outputDir = "output";
		
		// Create directory if it doesn't exist (mode 0755 = rwxr-xr-x)
		#ifdef _WIN32
			_mkdir(outputDir);
		#else
			mkdir(outputDir, 0755);
		#endif
	}
}