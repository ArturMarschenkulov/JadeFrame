#include "WinSystemManager.h"

#include <windows.h>
#include <winreg.h>
#include <intrin.h> // for "__cpuid()"

#include <iostream>

#include "../../uilts/Utils.h"

static auto get_primary_monitor_handle() -> HMONITOR {
	const POINT pt_zero = { 0, 0 };
	return ::MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
}
static auto calculate_dpi(HMODULE shCoreDll) -> int32_t {
	int32_t dpi_x = 96, dpi_y = 96;

	if (shCoreDll) {
		typedef HRESULT(STDAPICALLTYPE* GetDPIForMonitorProc)(HMONITOR hmonitor, UINT dpi_type, UINT* dpi_x, UINT* dpi_y);
		const GetDPIForMonitorProc GetDpiForMonitor = (GetDPIForMonitorProc)::GetProcAddress(shCoreDll, "GetDpiForMonitor");

		if (GetDpiForMonitor) {
			HMONITOR monitor = get_primary_monitor_handle();

			UINT x = 0, y = 0;
			HRESULT hr = GetDpiForMonitor(monitor, 0, &x, &y);
			if (SUCCEEDED(hr) && (x > 0) && (y > 0)) {
				dpi_x = (int32_t)x;
				dpi_y = (int32_t)y;
			}
		}
		::FreeLibrary(shCoreDll);
	}
	return (dpi_x + dpi_y) / 2;
}
static auto calculate_dpi() -> int32_t {
	if (const HMODULE shCoreDll = ::LoadLibraryW(L"Shcore.dll")) {
		return calculate_dpi(shCoreDll);
	}
	return 96;
}

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
		m_user_locale = from_wstring_to_string(buffer);

		GetSystemDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
		m_system_locale = from_wstring_to_string(buffer);

		GetComputerNameW(buffer, &size);
		m_computer_name = from_wstring_to_string(buffer);

		GetUserNameW(buffer, &size);
		m_user_name = from_wstring_to_string(buffer);
	}

	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&status);

		m_available_physical_memory = (int64_t)status.ullAvailPhys;
		m_total_physical_memory = (int64_t)status.ullTotalPhys;

		m_available_virtual_memory = (int64_t)status.ullAvailVirtual;
		m_total_virtual_memory = (int64_t)status.ullTotalVirtual;

	}
	{
		int args[4];
		__cpuid(args, 0x80000006);
		m_cache_line_size = args[2] & 0xFF;
	}
	{
		int cpuInfo[4] = { -1 };
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
		GetSystemInfo(&siSysInfo);
		m_page_size = siSysInfo.dwPageSize;
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

static auto bytes_to_string(const int64_t bytes) -> std::string {
	static const double TB = 1024ull * 1024ull * 1024ull * 1024ull;
	static const double GB = 1024ull * 1024ull * 1024ull;
	static const double MB = 1024ull * 1024ull;
	static const double KB = 1024ull;

	std::string result;
	if (bytes > TB) result = std::to_string(bytes / TB) + " TB";
	else if (bytes > GB) result = std::to_string(bytes / GB) + " GB";
	else if (bytes > MB) result = std::to_string(bytes / MB) + " MB";
	else if (bytes > KB) result = std::to_string(bytes / KB) + " KB";
	else result = std::to_string((float)bytes) + " bytes";

	return result;
}

auto Windows_SystemManager::log() const -> void {
	std::cout << "**********SYSTEM LOG**********" << std::endl;
	std::cout << "\tComputer Name: " << m_computer_name << std::endl;
	std::cout << "\tUser Name    : " << m_user_name << std::endl;
	std::cout << "\tCPU Name     : " << m_cpu_name << std::endl;




	std::cout << "\tAvailable RAM : " << bytes_to_string(m_available_physical_memory) << std::endl;
	std::cout << "\tTotal RAM     : " << bytes_to_string(m_total_physical_memory) << std::endl;
	std::cout << "\tAvailable RAM : " << bytes_to_string(m_available_virtual_memory) << std::endl;
	std::cout << "\tTotal RAM     : " << bytes_to_string(m_total_virtual_memory) << std::endl;
	std::cout << "******************************" << std::endl;
}