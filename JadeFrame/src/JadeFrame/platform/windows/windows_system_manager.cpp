#include "pch.h"
#include "windows_system_manager.h"

#include <Windows.h>
#include <winreg.h>
#include <intrin.h> // for "__cpuid()"

#include "JadeFrame/utils/utils.h"
namespace JadeFrame {
//static auto get_primary_monitor_handle() -> HMONITOR {
//	const POINT pt_zero = { 0, 0 };
//	return ::MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
//}
//static auto calculate_dpi(HMODULE shCoreDll) -> int32_t {
//	i32 dpi_x = 96, dpi_y = 96;
//
//	if (shCoreDll) {
//		typedef HRESULT(STDAPICALLTYPE* GetDPIForMonitorProc)(HMONITOR hmonitor, UINT dpi_type, UINT* dpi_x, UINT* dpi_y);
//		const GetDPIForMonitorProc GetDpiForMonitor = (GetDPIForMonitorProc)::GetProcAddress(shCoreDll, "GetDpiForMonitor");
//
//		if (GetDpiForMonitor) {
//			HMONITOR monitor = get_primary_monitor_handle();
//
//			UINT x = 0, y = 0;
//			HRESULT hr = GetDpiForMonitor(monitor, 0, &x, &y);
//			if (SUCCEEDED(hr) && (x > 0) && (y > 0)) {
//				dpi_x = (i32)x;
//				dpi_y = (i32)y;
//			}
//		}
//		::FreeLibrary(shCoreDll);
//	}
//	return (dpi_x + dpi_y) / 2;
//}
//static auto calculate_dpi() -> int32_t {
//	if (const HMODULE shCoreDll = ::LoadLibraryW(L"Shcore.dll")) {
//		return calculate_dpi(shCoreDll);
//	}
//	return 96;
//}

// Helper function to count set bits in the processor mask
DWORD count_set_bits(ULONG_PTR bit_mask) {
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bit_set_count = 0;
	ULONG_PTR bit_test = static_cast<ULONG_PTR>(1) << LSHIFT;
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i) {
		bit_set_count += ((bit_mask & bit_test) ? 1 : 0);
		bit_test /= 2;
	}

	return bit_set_count;
}

//static auto get_string_reg_key(HKEY hKey, const wchar_t* strValueName, std::string& strValue, const std::string& strDefaultValue) -> LONG {
//	strValue = strDefaultValue;
//	WCHAR szBuffer[512];
//	DWORD dwBufferSize = sizeof(szBuffer);
//	ULONG nError;
//	nError = RegQueryValueExW(hKey, strValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &dwBufferSize);
//	if (ERROR_SUCCESS == nError) {
//		strValue = szBuffer;
//	}
//	return nError;
//}

static auto get_DWORD_reg_key(HKEY hKey, const wchar_t* strValueName, DWORD& nValue, DWORD nDefaultValue) -> LONG {
	nValue = nDefaultValue;
	DWORD buffer_size(sizeof(DWORD));
	DWORD result(0);
	LONG error = RegQueryValueExW(hKey, strValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&result), &buffer_size);
	if (ERROR_SUCCESS == error) {
		nValue = result;
	}
	return error;
};
auto Windows_SystemManager::initialize() -> void {
	{
		TCHAR buffer[256] = L"";
		DWORD size = sizeof(buffer);


		GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
		m_user_locale = JadeFrame::from_wstring_to_string(buffer);

		GetSystemDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
		m_system_locale = JadeFrame::from_wstring_to_string(buffer);

		GetComputerNameW(buffer, &size);
		m_computer_name = JadeFrame::from_wstring_to_string(buffer);

		GetUserNameW(buffer, &size);
		m_user_name = JadeFrame::from_wstring_to_string(buffer);
	}

	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&status);

		m_available_physical_memory = (i64)status.ullAvailPhys;
		m_total_physical_memory = (i64)status.ullTotalPhys;

		m_available_virtual_memory = (i64)status.ullAvailVirtual;
		m_total_virtual_memory = (i64)status.ullTotalVirtual;

	}
	{
		i32 args[4];
		__cpuid(args, 0x80000006);
		m_cache_line_size = args[2] & 0xFF;
	}
	{
		i32 cpuInfo[4] = { -1 };
		char CPUBrandString[0x40];

		memset(CPUBrandString, 0, sizeof(CPUBrandString));

		__cpuid(cpuInfo, 0x80000002);
		memcpy(CPUBrandString, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000003);
		memcpy(CPUBrandString + 16, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000004);
		memcpy(CPUBrandString + 32, cpuInfo, sizeof(cpuInfo));
		m_cpu_name = CPUBrandString;
	}
	{
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
		DWORD return_length = 0;
		BOOL done = FALSE;
		while (!done) {
			DWORD rc = ::GetLogicalProcessorInformation(buffer, &return_length);
			if (FALSE == rc) {
				if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
					if (buffer) {
						free(buffer);
					}
					buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(return_length));
					if (buffer == nullptr) {
						return;
					}
				} else {
					return;
				}
			} else {
				done = TRUE;
			}
		}
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
		DWORD byte_offset = 0;
		DWORD processor_core_count = 0;
		DWORD logical_processor_count = 0;
		PCACHE_DESCRIPTOR cache;
		DWORD processor_L1_cache_size = 0;
		DWORD processor_L2_cache_size = 0;
		DWORD processor_L3_cache_size = 0;
		DWORD processor_package_count = 0;

		while (byte_offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= return_length) {
			switch (ptr->Relationship) {
				case RelationProcessorCore:

					processor_core_count++;

					// A hyper threaded core supplies more than one logical processor
					logical_processor_count += count_set_bits(ptr->ProcessorMask);
					break;

				case RelationCache:
					// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
					cache = &ptr->Cache;
					if (cache->Level == 1) {
						processor_L1_cache_size += cache->Size;
					} else if (cache->Level == 2) {
						processor_L2_cache_size += cache->Size;
					} else if (cache->Level == 3) {
						processor_L3_cache_size += cache->Size;
					}
					break;

				case RelationProcessorPackage:
					// Logical processors share a physical package
					processor_package_count++;
					break;
					//default:
					//	__debugbreak();
			}
			byte_offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}
		free(buffer);
		m_processor_package_count = processor_package_count;
		m_processor_core_count = processor_core_count;
		m_logical_processor_count = logical_processor_count;
		m_L1_cache_size = processor_L1_cache_size;
		m_L2_cache_size = processor_L2_cache_size;
		m_L3_cache_size = processor_L3_cache_size;

		//__debugbreak();
	}
	{
		SYSTEM_INFO siSysInfo;
		GetNativeSystemInfo(&siSysInfo);

		m_processor_architecture = siSysInfo.wProcessorArchitecture;
		m_page_size = siSysInfo.dwPageSize;
		m_minimum_application_address = siSysInfo.lpMinimumApplicationAddress;
		m_maximum_application_address = siSysInfo.lpMaximumApplicationAddress;
		m_active_processor_mask = siSysInfo.dwActiveProcessorMask;
		m_number_of_processors = siSysInfo.dwNumberOfProcessors;
		m_allocation_granularity = siSysInfo.dwAllocationGranularity;
		m_processor_level = siSysInfo.wProcessorLevel;
		m_processor_revision = siSysInfo.wProcessorRevision;
	}

	{
		HKEY hKey;

		/*LONG lRes =*/ RegOpenKeyExW(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ, &hKey);
		DWORD current_major_version_number;
		DWORD current_minor_version_number;
		get_DWORD_reg_key(hKey, TEXT("CurrentMajorVersionNumber"), current_major_version_number, 0);
		get_DWORD_reg_key(hKey, TEXT("CurrentMinorVersionNumber"), current_minor_version_number, 0);

		m_window_version_major = current_major_version_number;
		m_window_version_minor = current_minor_version_number;
	}
}

static auto bytes_to_string(const u64 bytes) -> std::string {
	constexpr const f64 TB = static_cast<f64>(1024_u64 * 1024_u64 * 1024_u64 * 1024_u64);
	constexpr const f64 GB = static_cast<f64>(1024_u64 * 1024_u64 * 1024_u64);
	constexpr const f64 MB = static_cast<f64>(1024_u64 * 1024_u64);
	constexpr const f64 KB = static_cast<f64>(1024_u64);

	std::string result;
	if (bytes > TB) result = std::to_string(bytes / TB) + " TB";
	else if (bytes > GB) result = std::to_string(bytes / GB) + " GB";
	else if (bytes > MB) result = std::to_string(bytes / MB) + " MB";
	else if (bytes > KB) result = std::to_string(bytes / KB) + " KB";
	else result = std::to_string((f32)bytes) + " bytes";

	return result;
}
static auto win32_get_processor_architecture_string(u16 t) -> std::string {
	std::string result;
	switch (t) {
		case PROCESSOR_ARCHITECTURE_AMD64: result = "x64 (AMD or Intel)"; break;
		case PROCESSOR_ARCHITECTURE_ARM: result = "ARM"; break;
		case PROCESSOR_ARCHITECTURE_ARM64: result = "ARM64"; break;
		case PROCESSOR_ARCHITECTURE_IA64: result = "Intel Itanium-based"; break;
		case PROCESSOR_ARCHITECTURE_INTEL: result = "x86"; break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN: result = "Unknown architecture"; break;
	}
	return result;
}
auto Windows_SystemManager::log() const -> void {
	std::cout
		<< "**********SYSTEM LOG**********" << "\n"
		<< "\tComputer Name: " << m_computer_name << "\n"
		<< "\tUser Name    : " << m_user_name << "\n"
		<< "\tWin Ver Maj  : " << m_window_version_major << "\n"
		<< "\tWin Ver Min  : " << m_window_version_minor << "\n"
		<< "**********CPU DATA**********" << "\n"
		<< "\tCPU Name     : " << m_cpu_name << "\n"
		<< "\tCache Line Size: " << m_cache_line_size << "\n"
		<< "\tL1 Cache Size: " << bytes_to_string(m_L1_cache_size) << "\n"
		<< "\tL2 Cache Size: " << bytes_to_string(m_L2_cache_size) << "\n"
		<< "\tL3 Cache Size: " << bytes_to_string(m_L3_cache_size) << "\n"
		<< "\tCPU Package Count: " << m_processor_package_count << "\n"
		<< "\tCPU Core Count   : " << m_processor_core_count << "\n"
		<< "\tCPU Logical Count: " << m_logical_processor_count << "\n"
		<< "\tPage Size    : " << bytes_to_string(m_page_size) << "\n"
		<< "\tCPU Architecture: " << win32_get_processor_architecture_string(m_processor_architecture) << "\n"

		<< "**********RAM DATA**********" << "\n"
		<< "\tAvailable RAM : " << bytes_to_string(m_available_physical_memory) << "\n"
		<< "\tTotal RAM     : " << bytes_to_string(m_total_physical_memory) << "\n"
		<< "\tAvailable RAM : " << bytes_to_string(m_available_virtual_memory) << "\n"
		<< "\tTotal RAM     : " << bytes_to_string(m_total_virtual_memory) << "\n"
		<< "******************************" << std::endl;
}
}