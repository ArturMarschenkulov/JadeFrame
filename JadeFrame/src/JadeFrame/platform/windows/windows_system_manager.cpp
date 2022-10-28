#include "pch.h"
#include "windows_system_manager.h"
#include "windows_shared.h"
#include "windows_window.h"

#include "JadeFrame/utils/option.h"

#include <Windows.h>
#include <winreg.h>
#include <intrin.h> // for "__cpuid()"
#include <tlhelp32.h>
#include "spdlog/fmt/xchar.h"

#include "JadeFrame/utils/utils.h"
namespace JadeFrame {

// namespace platform {
namespace win32 {
DWORD count_set_bits(ULONG_PTR bit_mask) {
    DWORD     LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD     bit_set_count = 0;
    ULONG_PTR bit_test = static_cast<ULONG_PTR>(1) << LSHIFT;
    DWORD     i;

    for (i = 0; i <= LSHIFT; ++i) {
        bit_set_count += ((bit_mask & bit_test) ? 1 : 0);
        bit_test /= 2;
    }

    return bit_set_count;
}
// auto get_heaps() -> std::vector<int> {}


struct ProcessEntry {
    u32     size;
    u32     usage;
    u32     process_id;
    u64     default_heap_id;
    u32     module_id;
    u32     thread_count;
    u32     paranet_process_id;
    i64     priority;
    u32     flags;
    wchar_t path[MAX_PATH];
};

struct ModuleEntry {
    u64       size;
    u64       module_id;
    u64       process_id;
    u64       global_usage_count;
    u64       process_usage_count;
    u8*       base_adress;
    u64       base_size;
    HINSTANCE instance;
    wchar_t   module_size[MAX_MODULE_NAME32 + 1];
    wchar_t   path[MAX_PATH];
};
static auto to_process_entry(const PROCESSENTRY32W& entry) -> ProcessEntry {
    ProcessEntry result;
    result.size = entry.dwSize;
    result.usage = entry.cntUsage;
    result.process_id = entry.th32ProcessID;
    result.default_heap_id = entry.th32DefaultHeapID;
    result.module_id = entry.th32ModuleID;
    result.thread_count = entry.cntThreads;
    result.paranet_process_id = entry.th32ParentProcessID;
    result.priority = entry.pcPriClassBase;
    result.flags = entry.dwFlags;
    wcscpy_s(result.path, entry.szExeFile);
    return result;
}
static auto to_module_entry(const MODULEENTRY32W& module_entry) -> ModuleEntry {
    ModuleEntry entry;
    entry.size = module_entry.dwSize;
    entry.module_id = module_entry.th32ModuleID;
    entry.process_id = module_entry.th32ProcessID;
    entry.global_usage_count = module_entry.GlblcntUsage;
    entry.process_usage_count = module_entry.ProccntUsage;
    entry.base_adress = module_entry.modBaseAddr;
    entry.base_size = module_entry.modBaseSize;
    entry.instance = module_entry.hModule;
    wcscpy_s(entry.module_size, module_entry.szModule);
    wcscpy_s(entry.path, module_entry.szExePath);
    return entry;
}

auto get_processes() -> std::vector<ProcessEntry> {
    std::vector<ProcessEntry> processes;

    ::HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        ::PROCESSENTRY32W process_entry;
        process_entry.dwSize = sizeof(process_entry);
        ::BOOL success = ::Process32FirstW(snapshot, &process_entry);
        if (success == TRUE) {
            do {
                processes.emplace_back();
                processes.back() = to_process_entry(process_entry);
                // processes.back() = *(ProcessEntry*)&process_entry;
                // processes.back() = *reinterpret_cast<ProcessEntry*>(&process_entry);
            } while (::Process32NextW(snapshot, &process_entry));
        }
        ::CloseHandle(snapshot);
    }
    return processes;
}
auto get_modules() -> std::vector<ModuleEntry> {
    std::vector<ModuleEntry> modules;
    u32 flags = TH32CS_INHERIT | TH32CS_SNAPALL | TH32CS_SNAPHEAPLIST | TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32 |
                TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD;
    ::HANDLE snapshot = ::CreateToolhelp32Snapshot(flags, 0);
    if (snapshot == INVALID_HANDLE_VALUE) { return modules; }

    ::MODULEENTRY32W module_entry;
    module_entry.dwSize = sizeof(module_entry);
    ::BOOL success = ::Module32FirstW(snapshot, &module_entry);
    if (success == TRUE) {
        do {
            modules.emplace_back();
            modules.back() = to_module_entry(module_entry);
            // modules.back() = *reinterpret_cast<ModuleEntry*>(&module_entry);
        } while (::Module32NextW(snapshot, &module_entry));
    }
    ::CloseHandle(snapshot);
    return modules;
}

static auto get_DWORD_reg_key(HKEY hKey, const wchar_t* strValueName, DWORD& nValue, DWORD nDefaultValue) -> LONG {
    nValue = nDefaultValue;
    DWORD buffer_size(sizeof(DWORD));
    DWORD result(0);
    LONG  error =
        RegQueryValueExW(hKey, strValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&result), &buffer_size);
    if (ERROR_SUCCESS == error) { nValue = result; }
    return error;
};

static auto get_processor_name() -> std::string {
    const wchar_t* id = L"Hardware\\Description\\System\\CentralProcessor\\0";
    HKEY           hKey;

    LONG error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, (LPCWSTR)id, 0, KEY_QUERY_VALUE, &hKey);
    if (ERROR_SUCCESS != error) {}

    WCHAR buffer[256];
    DWORD buffer_len = 256;
    DWORD vtype = REG_SZ;
    error = RegQueryValueExW(hKey, L"ProcessorNameString", nullptr, &vtype, (LPBYTE)buffer, &buffer_len);
    return "";
}

auto test() -> void {
    // TODO: Check whether our assumptions are correct.
    static_assert(sizeof(DWORD) == 4, "DWORD is not 32-bit");         // u32
    static_assert(sizeof(DWORD_PTR) == 8, "DWORD_PTR is not 64-bit"); // u64
    static_assert(sizeof(ULONG_PTR) == 8, "ULONG_PTR is not 64-bit"); // u64
    static_assert(sizeof(LONG) == 4, "LONG is not 32-bit");           // i32
    static_assert(sizeof(WCHAR) == 2, "WCHAR is not 16-bit");         // u16
}
// static auto get_primary_monitor_handle() -> HMONITOR {
//	const POINT pt_zero = { 0, 0 };
//	return ::MonitorFromPoint(pt_zero, MONITOR_DEFAULTTOPRIMARY);
// }
// static auto calculate_dpi(HMODULE shCoreDll) -> int32_t {
//	i32 dpi_x = 96, dpi_y = 96;
//
//	if (shCoreDll) {
//		typedef HRESULT(STDAPICALLTYPE* GetDPIForMonitorProc)(HMONITOR hmonitor, UINT dpi_type, UINT* dpi_x,
// UINT* dpi_y); 		const GetDPIForMonitorProc GetDpiForMonitor =
// (GetDPIForMonitorProc)::GetProcAddress(shCoreDll, "GetDpiForMonitor");
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
// }
// static auto calculate_dpi() -> int32_t {
//	if (const HMODULE shCoreDll = ::LoadLibraryW(L"Shcore.dll")) {
//		return calculate_dpi(shCoreDll);
//	}
//	return 96;
// }

// static auto get_string_reg_key(HKEY hKey, const wchar_t* strValueName, std::string& strValue, const std::string&
// strDefaultValue) -> LONG { 	strValue = strDefaultValue; 	WCHAR szBuffer[512]; 	DWORD dwBufferSize =
// sizeof(szBuffer);
//	ULONG nError;
//	nError = RegQueryValueExW(hKey, strValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer),
//&dwBufferSize); 	if (ERROR_SUCCESS == nError) { 		strValue = szBuffer;
//	}
//	return nError;
// }


auto SystemManager::request_window(IWindow::Desc desc) -> IWindow* {
    m_windows[m_window_counter] = std::make_unique<Window>(desc);
    m_window_counter++;
    return m_windows[m_window_counter - 1].get();
}


#define USE_OPTION_TYPE 1
#if USE_OPTION_TYPE
static auto query_performance_frequency() -> Option<u64> {
    u64 frequency;
    if (::QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
        return Option<u64>(std::move(frequency));
    } else {
        return Option<u64>();
    }
}
static auto query_performance_counter() -> Option<u64> {
    u64 counter;
    if (::QueryPerformanceCounter((LARGE_INTEGER*)&counter)) {
        return Option<u64>(std::move(counter));
    } else {
        return Option<u64>();
    }
}
#endif

auto SystemManager::calc_elapsed() -> f64 {
    f64 current = this->get_time();
    f64 update = current - time.previous;
    time.previous = current;
    return update;
}

auto SystemManager::frame_control(f64 delta_time) -> void {
    // Frame time control system
    f64 current = this->get_time();
    f64 draw = current - time.previous;
    time.previous = current;

    f64 frame = delta_time + draw;

    if (frame < time.target) {
        ::Sleep(static_cast<u32>((time.target - frame) * 1000.0));
        f64 current = this->get_time();
        f64 time_wait = current - time.previous;
        time.previous = current;
        frame += time_wait;
    }
}

auto SystemManager::set_target_FPS(f64 FPS) -> void {
    max_FPS = static_cast<f32>(FPS);
    time.target = 1 / (f64)FPS;
}

auto SystemManager::get_time() const -> f64 {
#if USE_OPTION_TYPE
    const u64 counter = query_performance_counter().unwrap_unchecked() - m_offset;
    const u64 frequency = m_frequency;
    return static_cast<f64>(counter) / frequency;
#else
    u64 counter;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&counter);
    const u64 frequency = m_frequency;

    return static_cast<f64>(counter) / frequency;
#endif
}
auto SystemManager::initialize() -> void {

    // time stuff
    timeBeginPeriod(1);

#if USE_OPTION_TYPE
    Option<u64> frequency = query_performance_frequency();
    if (frequency.is_some()) {
        m_has_performance_counter = true;
        m_frequency = frequency.unwrap_unchecked();
    } else {
        m_has_performance_counter = false;
        m_frequency = 1000;
    }
    m_offset = query_performance_counter().unwrap_unchecked();
#else
    u64 frequency;
    if (::QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
        m_has_performance_counter = true;
        m_frequency = frequency;
    } else {
        m_has_performance_counter = false;
        m_frequency = 1000;
    }
    ::QueryPerformanceCounter((LARGE_INTEGER*)&m_offset);
#endif

    // time stuff end
    m_instance = ::GetModuleHandleW(NULL);
    Logger::debug("Initializing Windows System Manager");
    if (false) {
        std::vector<ModuleEntry>  modules = get_modules();
        std::vector<ProcessEntry> processes = get_processes();

        Logger::debug("There are {} modules loaded", modules.size());
        for (auto& module : modules) {
            char output[260];
            sprintf(output, "%ws", module.path);
            Logger::debug("\tModule: {}", output);
        }
        Logger::debug("There are {} processes running", processes.size());
        for (auto& process : processes) {
            char output[260];
            sprintf(output, "%ws", process.path);
            Logger::debug("\tProcess: {}", output);
        }
    }
    {

        u32 flags = TH32CS_INHERIT | TH32CS_SNAPALL | TH32CS_SNAPHEAPLIST | TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32 |
                    TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD;
        ::HANDLE snapshot = ::CreateToolhelp32Snapshot(flags, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            ::MODULEENTRY32W module_entry;
            module_entry.dwSize = sizeof(module_entry);
            ::BOOL success = ::Module32FirstW(snapshot, &module_entry);
            if (success == TRUE) {
                do {
                    m_modules.emplace_back();
                    m_modules.back().m_name = from_wstring_to_string(module_entry.szModule);
                    m_modules.back().m_path = from_wstring_to_string(module_entry.szExePath);
                    m_modules.back().m_id = module_entry.th32ModuleID;
                    m_modules.back().m_process_id = module_entry.th32ProcessID;
                    m_modules.back().m_global_usage_count = module_entry.GlblcntUsage;
                    m_modules.back().m_process_usage_count = module_entry.ProccntUsage;
                } while (::Module32NextW(snapshot, &module_entry));
            }
            ::CloseHandle(snapshot);
        }
    }
    {
        ::TCHAR buffer[256] = L"";
        ::DWORD size = sizeof(buffer);


        ::GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
        m_user_locale = from_wstring_to_string(buffer);

        ::GetSystemDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
        m_system_locale = from_wstring_to_string(buffer);

        ::GetComputerNameW(buffer, &size);
        m_computer_name = from_wstring_to_string(buffer);

        ::GetUserNameW(buffer, &size);
        m_user_name = from_wstring_to_string(buffer);
    }

    {
        ::MEMORYSTATUSEX status;
        status.dwLength = sizeof(::MEMORYSTATUSEX);
        ::GlobalMemoryStatusEx(&status);
        m_available_physical_memory = (i64)status.ullAvailPhys;
        m_total_physical_memory = (i64)status.ullTotalPhys;

        m_available_virtual_memory = (i64)status.ullAvailVirtual;
        m_total_virtual_memory = (i64)status.ullTotalVirtual;
    }
    {
        i32 args[4];
        ::__cpuid(args, 0x80000006);
        m_cache_line_size = args[2] & 0xFF;
    }
    {
        i32  cpu_info[4] = {-1};
        char CPU_brand_string[0x40];

        ::memset(CPU_brand_string, 0, sizeof(CPU_brand_string));

        ::__cpuid(cpu_info, 0x80000002);
        ::memcpy(CPU_brand_string, cpu_info, sizeof(cpu_info));

        ::__cpuid(cpu_info, 0x80000003);
        ::memcpy(CPU_brand_string + 16, cpu_info, sizeof(cpu_info));

        ::__cpuid(cpu_info, 0x80000004);
        ::memcpy(CPU_brand_string + 32, cpu_info, sizeof(cpu_info));
        m_cpu_name = CPU_brand_string;
    }
    {
        ::PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
        ::DWORD                                 return_length = 0;
        ::BOOL                                  done = FALSE;
        while (!done) {
            DWORD rc = ::GetLogicalProcessorInformation(buffer, &return_length);
            if (FALSE == rc) {
                if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    if (buffer) { ::free(buffer); }
                    buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(return_length));
                    if (buffer == nullptr) { return; }
                } else {
                    return;
                }
            } else {
                done = TRUE;
            }
        }
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
        DWORD                                 byte_offset = 0;
        DWORD                                 processor_core_count = 0;
        DWORD                                 logical_processor_count = 0;
        PCACHE_DESCRIPTOR                     cache;
        DWORD                                 processor_L1_cache_size = 0;
        DWORD                                 processor_L2_cache_size = 0;
        DWORD                                 processor_L3_cache_size = 0;
        DWORD                                 processor_package_count = 0;

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
                    // default:
                    //	__debugbreak();
                case RelationNumaNode:
                    // Non-NUMA systems report a single record of this type.
                    break;
                case RelationGroup:
                    // We're not sure how to interpret this, so don't.
                    break;
                case RelationAll:
                    // We're not sure how to interpret this, so don't.
                    break;
                default: break;
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

        /*LONG lRes =*/RegOpenKeyExW(
            HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ, &hKey);
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
auto SystemManager::log() const -> void {
    std::cout << "**********SYSTEM LOG**********"
              << "\n"
              << "\tComputer Name: " << m_computer_name << "\n"
              << "\tUser Name    : " << m_user_name << "\n"
              << "\tWin Ver Maj  : " << m_window_version_major << "\n"
              << "\tWin Ver Min  : " << m_window_version_minor << "\n"
              << "**********CPU DATA**********"
              << "\n"
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

              << "**********RAM DATA**********"
              << "\n"
              << "\tAvailable RAM : " << bytes_to_string(m_available_physical_memory) << "\n"
              << "\tTotal RAM     : " << bytes_to_string(m_total_physical_memory) << "\n"
              << "\tAvailable RAM : " << bytes_to_string(m_available_virtual_memory) << "\n"
              << "\tTotal RAM     : " << bytes_to_string(m_total_virtual_memory) << "\n"
              << "******************************" << std::endl;
}

} // namespace win32
} // namespace JadeFrame