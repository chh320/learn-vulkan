#pragma once

#include <cstdint>
#include <string>

namespace vk {
	struct WindowConfig final
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool CursorDisabled;
		bool Fullscreen;
		bool Resizable;
	};
}