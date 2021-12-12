#include "pch.h"
#include "logger.h"

//#include "exter/spdlog/sinks/stdout_color_sinks.h"
//#include "exter/spdlog/sinks/basic_file_sink.h"


namespace JadeFrame {
//std::shared_ptr<spdlog::logger> Logger::s_core;


auto Logger::init() -> void {
	//std::vector<spdlog::sink_ptr> jf_sinks =
	//{
	//	std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
	//	std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/JadeFrame.log", true)
	//};

	//jf_sinks[0]->set_pattern("%^[%T] %n: %v%$");
	//s_core = std::make_shared<spdlog::logger>("JadeFrame", jf_sinks.begin(), jf_sinks.end());
}
auto Logger::deinit() -> void {
	//s_core.reset();
	//spdlog::drop_all();
}
}

