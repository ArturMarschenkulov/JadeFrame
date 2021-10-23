#pragma once
#include "logger.h"


namespace JadeFrame {


#define JF_ASSERT(expr, text) do {				\
	if (expr) {									\
	} else {									\
		std::string t = std::format("{}: {}: {}", __FILE__, __LINE__, std::string(text));\
		Logger::log(Logger::LEVEL::ERR, t.c_str());	\
		__debugbreak();							\
	}											\
} while (0)



}