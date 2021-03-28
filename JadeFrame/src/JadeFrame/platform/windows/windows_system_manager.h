
#include <string>

class Windows_SystemManager {
public:
	auto initialize() -> void;
	auto log() const -> void;

public:
	std::string m_user_locale;
	std::string m_system_locale;
	std::string m_computer_name;
	std::string m_user_name;

	std::string m_cpu_name;

	int64_t m_available_physical_memory;
	int64_t m_total_physical_memory;
	int64_t m_available_virtual_memory;
	int64_t m_total_virtual_memory;

	int64_t m_window_version_major;
	int64_t m_window_version_minor;

	uint32_t m_cache_line_size;
	uint32_t m_L1_cache_size;
	uint32_t m_L2_cache_size;
	uint32_t m_L3_cache_size;
	uint32_t m_processor_package_count;
	uint32_t m_processor_core_count;
	uint32_t m_logical_processor_count;
	uint32_t m_page_size;

};