#pragma once
#include "logger.h"


namespace JadeFrame {


#define JF_ASSERT(expr, text) do {				\
	if (expr) {									\
	} else {									\
		std::string t = fmt::v8::format("{}: {}: {}", __FILE__, __LINE__, std::string(text));\
		Logger::err(t.c_str());	\
		assert(false);							\
	}											\
} while (0)



}