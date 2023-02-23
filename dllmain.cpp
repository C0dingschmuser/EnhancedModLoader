#define WIN32_LEAN_AND_MEAN  

#include <Windows.h>
#include <string>
#include <filesystem>
#include <fstream>
#include "include/coreclr_delegates.h";
#include "include/hostfxr.h";

#pragma comment(linker,"/export:AvCreateTaskIndex=C:\\Windows\\System32\\avrt.AvCreateTaskIndex,@1")
#pragma comment(linker,"/export:AvQuerySystemResponsiveness=C:\\Windows\\System32\\avrt.AvQuerySystemResponsiveness,@2")
#pragma comment(linker,"/export:AvQueryTaskIndexValue=C:\\Windows\\System32\\avrt.AvQueryTaskIndexValue,@3")
#pragma comment(linker,"/export:AvRevertMmThreadCharacteristics=C:\\Windows\\System32\\avrt.AvRevertMmThreadCharacteristics,@4")
#pragma comment(linker,"/export:AvRtCreateThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtCreateThreadOrderingGroup,@5")
#pragma comment(linker,"/export:AvRtCreateThreadOrderingGroupExA=C:\\Windows\\System32\\avrt.AvRtCreateThreadOrderingGroupExA,@6")
#pragma comment(linker,"/export:AvRtCreateThreadOrderingGroupExW=C:\\Windows\\System32\\avrt.AvRtCreateThreadOrderingGroupExW,@7")
#pragma comment(linker,"/export:AvRtDeleteThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtDeleteThreadOrderingGroup,@8")
#pragma comment(linker,"/export:AvRtJoinThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtJoinThreadOrderingGroup,@9")
#pragma comment(linker,"/export:AvRtLeaveThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtLeaveThreadOrderingGroup,@10")
#pragma comment(linker,"/export:AvRtWaitOnThreadOrderingGroup=C:\\Windows\\System32\\avrt.AvRtWaitOnThreadOrderingGroup,@11")
#pragma comment(linker,"/export:AvSetMmMaxThreadCharacteristicsA=C:\\Windows\\System32\\avrt.AvSetMmMaxThreadCharacteristicsA,@12")
#pragma comment(linker,"/export:AvSetMmMaxThreadCharacteristicsW=C:\\Windows\\System32\\avrt.AvSetMmMaxThreadCharacteristicsW,@13")
#pragma comment(linker,"/export:AvSetMmThreadCharacteristicsA=C:\\Windows\\System32\\avrt.AvSetMmThreadCharacteristicsA,@14")
#pragma comment(linker,"/export:AvSetMmThreadCharacteristicsW=C:\\Windows\\System32\\avrt.AvSetMmThreadCharacteristicsW,@15")
#pragma comment(linker,"/export:AvSetMmThreadPriority=C:\\Windows\\System32\\avrt.AvSetMmThreadPriority,@16")
#pragma comment(linker,"/export:AvSetMultimediaMode=C:\\Windows\\System32\\avrt.AvSetMultimediaMode,@17")
#pragma comment(linker,"/export:AvTaskIndexYield=C:\\Windows\\System32\\avrt.AvTaskIndexYield,@18")
#pragma comment(linker,"/export:AvTaskIndexYieldCancel=C:\\Windows\\System32\\avrt.AvTaskIndexYieldCancel,@19")
#pragma comment(linker,"/export:AvThreadOpenTaskIndex=C:\\Windows\\System32\\avrt.AvThreadOpenTaskIndex,@20")

//Credits to StackOverflowExcept1on for this injector
//https://github.com/StackOverflowExcept1on/net-core-injector

class Module
{
public:
    static void* getBaseAddress(const char* library)
    {
#ifdef _WIN32
        auto base = GetModuleHandleA(library);
#else
        auto base = dlopen(library, RTLD_LAZY);
#endif
        return reinterpret_cast<void*>(base);
    }

    static void* getExportByName(void* module, const char* name)
    {
#ifdef _WIN32
        auto address = GetProcAddress((HMODULE)module, name);
#else
        auto address = dlsym(module, name);
#endif
        return reinterpret_cast<void*>(address);
    }

    template<typename T>
    static T getFunctionByName(void* module, const char* name)
    {
        return reinterpret_cast<T>(getExportByName(module, name));
    }
};

enum class InitializeResult : uint32_t
{
    Success,
    HostFxrLoadError,
    InitializeRuntimeConfigError,
    GetRuntimeDelegateError,
    EntryPointError,
};

InitializeResult LoadDll(const char_t* runtime_config_path, const char_t* assembly_path, const char_t* type_name, const char_t* method_name)
{
    /// Get module base address
#ifdef _WIN32
    auto libraryName = "hostfxr.dll";
#else
    auto libraryName = "libhostfxr.so";
#endif
    void* module = Module::getBaseAddress(libraryName);
    if (!module)
    {
        return InitializeResult::HostFxrLoadError;
    }

    /// Obtaining useful exports
    auto hostfxr_initialize_for_runtime_config_fptr =
        Module::getFunctionByName<hostfxr_initialize_for_runtime_config_fn>(module, "hostfxr_initialize_for_runtime_config");

    auto hostfxr_get_runtime_delegate_fptr =
        Module::getFunctionByName<hostfxr_get_runtime_delegate_fn>(module, "hostfxr_get_runtime_delegate");

    auto hostfxr_close_fptr =
        Module::getFunctionByName<hostfxr_close_fn>(module, "hostfxr_close");

    /// Load runtime config
    hostfxr_handle ctx = nullptr;
    int rc = hostfxr_initialize_for_runtime_config_fptr(runtime_config_path, nullptr, &ctx);

    /// Success_HostAlreadyInitialized = 0x00000001
    /// @see https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md
    if (rc != 1 || ctx == nullptr)
    {
        hostfxr_close_fptr(ctx);
        return InitializeResult::InitializeRuntimeConfigError;
    }

    /// From docs: native function pointer to the requested runtime functionality
    void* delegate = nullptr;
    int ret = hostfxr_get_runtime_delegate_fptr(ctx, hostfxr_delegate_type::hdt_load_assembly_and_get_function_pointer,
        &delegate);

    if (ret != 0 || delegate == nullptr)
    {
        return InitializeResult::GetRuntimeDelegateError;
    }

    /// `void *` -> `load_assembly_and_get_function_pointer_fn`, undocumented???
    auto load_assembly_fptr = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(delegate);

    typedef void (CORECLR_DELEGATE_CALLTYPE* custom_entry_point_fn)();
    custom_entry_point_fn custom = nullptr;

    ret = load_assembly_fptr(assembly_path, type_name, method_name, UNMANAGEDCALLERSONLY_METHOD, nullptr,
        (void**)&custom);

    if (ret != 0 || custom == nullptr)
    {
        return InitializeResult::EntryPointError;
    }

    custom();

    hostfxr_close_fptr(ctx);
}

std::string GetExecutableDirectory()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");

    return std::string(buffer).substr(0, pos);
}

DWORD WINAPI dllThread(HMODULE hModule)
{
    DWORD dwExit = 0;

    std::string exePath = GetExecutableDirectory();

    std::string helperPath = exePath.substr(0, exePath.find("common")) +
        "workshop\\content\\799600\\2937578771\\leave_this_folder_here\\";

    std::string configPath = exePath + "\\eml_config.ini";
    std::string modsPath = exePath + "\\eml_mods.ini";
    std::string lockPath = modsPath + ".lock";

    //delete old lockfile if exists
    if (std::filesystem::exists(lockPath))
    {
        std::filesystem::remove(lockPath);
    }

    if (std::filesystem::exists(configPath))
    {
        //load path from config file if it exists
        std::ifstream file(configPath);

        std::string token = "EML_HelperPath";

        for (std::string line; getline(file, line); )
        {
            if (line.find(token) != std::string::npos)
            {
                helperPath = line.substr(line.find(token) + token.length() + 1);
            }
        }
    }
    else
    {
        std::ofstream file("eml_config.ini", std::ios::trunc);

        if (file.is_open())
        {
            file << "EML_HelperPath=" << helperPath << std::endl;
            file.close();
        }
    }


    std::string s_config = helperPath + "EML_Helper.runtimeconfig.json";
    std::wstring t_config = std::wstring(s_config.begin(), s_config.end());
    const char_t* config = t_config.c_str();

    std::string s_dll = helperPath + "EML_Helper.dll";
    std::wstring t_dll = std::wstring(s_dll.begin(), s_dll.end());
    const char_t* dll = t_dll.c_str();

    const char_t* typeName = L"EML_Helper.Main, EML_Helper";
    const char_t* methodName = L"InitializePatches";

    //Give the game time to initialize
    Sleep(1000);

    if (!std::filesystem::exists(s_dll))
    {
        MessageBoxA(NULL, "EML_Helper.dll not found!\nCheck eml_config.ini in your Cosmoteer Install directory", "Error", MB_OK | MB_ICONERROR);
		return 0;
    }

    //Loads helper dll
    LoadDll(config, dll, typeName, methodName);

    //Wait for mod list
    while (!std::filesystem::exists(lockPath))
    {
        Sleep(250);
    }

    if (std::filesystem::exists(modsPath))
    {
        std::ifstream file(modsPath);

        for (std::string line; getline(file, line); )
        {
            std::string s_dllPath = line;

            if (std::filesystem::exists(s_dllPath))
            {
                std::wstring t_dllPath = std::wstring(s_dllPath.begin(), s_dllPath.end());
                const char_t* dllPath = t_dllPath.c_str();

                //get filename without path and without file extension
                std::string filename = line.substr(line.find_last_of("\\/") + 1);
                filename = filename.substr(0, filename.find_last_of("."));

                std::string s_typeName = filename + ".Main, " + filename;
                std::wstring t_typeName = std::wstring(s_typeName.begin(), s_typeName.end());
                const char_t* typeName = t_typeName.c_str();

                LoadDll(config, dllPath, typeName, methodName);
            }
        }
    }

    FreeLibraryAndExitThread(hModule, 0);
    return dwExit;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)dllThread, hModule, 0, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

