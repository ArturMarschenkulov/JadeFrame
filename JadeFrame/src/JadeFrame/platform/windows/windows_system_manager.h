#pragma once
#include "JadeFrame/defines.h"
#include <string>
namespace JadeFrame {
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

	i64 m_available_physical_memory;
	i64 m_total_physical_memory;
	i64 m_available_virtual_memory;
	i64 m_total_virtual_memory;

	i64 m_window_version_major;
	i64 m_window_version_minor;

	u32 m_cache_line_size;
	u32 m_L1_cache_size;
	u32 m_L2_cache_size;
	u32 m_L3_cache_size;
	u32 m_processor_package_count;
	u32 m_processor_core_count;
	u32 m_logical_processor_count;

	u16 m_processor_architecture;
	u32 m_page_size;
	void* m_minimum_application_address;
	void* m_maximum_application_address;
	u64 m_active_processor_mask;
	u32 m_number_of_processors;
	u32 m_allocation_granularity;
	u16 m_processor_level;
	u16 m_processor_revision;
};
}