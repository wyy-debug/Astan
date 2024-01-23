#pragma once

#include "Astan/Core/Buffer.h"

namespace Astan {

	class FileSystem
	{
	public:
		// TODO: move to FileSystem class
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};

}