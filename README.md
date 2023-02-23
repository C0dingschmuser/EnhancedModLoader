# EnhancedModLoader for Cosmoteer
### Automatically loads c# mod dlls from Steam Workshop Mods - Integrates with ingame mod manager.

| [Installation](#----installation----) - [How it works](#----how-it-works----) - [Developing C# Mods](#----developing-c-mods----) - [Troubleshooting](#----troubleshooting----) - [License](#----license----)|
:----------------------------------------------------------: |

### --- Installation ---
1. Subscribe to this Mod on the [Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=2937868590)  
2. Navigate to your Cosmoteer Workshop directory.  
   Usually it's ```C:\Program Files (x86)\Steam\steamapps\workshop\content\799600\2937868590```  
   Note: ```799600 is the ID for Cosmoteer, 2937868590 is the ID for this Mod```  
3. **Copy AVRT.dll** from the Mod Folder to your Cosmoteer "Bin" path. This is the path where the Cosmoteer.exe lies.  
   Example: ```C:\Program Files (x86)\Steam\steamapps\common\Cosmoteer\Bin```
4. You're good to go! Now you can just subscribe to any Workshop EML Mod and it will load automatically  
(if it's enabled in the ingame mod manager).
5. (Optional) If you want to test if everything works you can subscribe to the [EML Test Mod](https://steamcommunity.com/sharedfiles/filedetails/?id=2937811110).
   This simple Mod will show a Message Box when you start to play Cosmoteer.

### --- How it works ---

AVRT.dll is a Library that Cosmoteer tries to load from it's local path that does not exist. Conveniently the local path is the first one that is being searched for this dll before searching in windows system folders and eventually loading it from there. I'm utilizing this to my advantage by copying the functionality of the original AVRT.dll + adding my own. This is called [Dll Hijacking](https://book.hacktricks.xyz/windows-hardening/windows-local-privilege-escalation/dll-hijacking).

I'm using a slightly modified version of [StackOverflowExcept1on's .net core injector](https://github.com/StackOverflowExcept1on/net-core-injector) to manually load a c# helper dll, EML_Helper.dll, to get all current enabled mods (that contain dlls) and pass them back to the original c++ dll which then loads them.

### --- Developing C# Mods ---
If you want to make your own c# mod dll, this will get you started:
- Use target SDK 6.0.403 (Cosmoteer & EML both use this version), specify this in a global.json in your Project Directory. [See more](https://learn.microsoft.com/de-de/dotnet/core/tools/global-json)
- Use Runtime Framework Version 6.0.11
- Use Target Framework net6.0-windows
- Allow Unsafe Blocks

Important:

- Entry Point namespace **MUST** have the same name as the dll file (but without the .dll)
- Entry Point class **MUST** be named Main
- Entry Point method **MUST** be named InitializePatches and have the ```[UnmanagedCallersOnly]``` attribute
- Use [Harmony](https://github.com/pardeike/Harmony) for patching methods
- Add Assembly References for Cosmoteer.dll and HalflingCore.dll from your Cosmoteer Bin Path  
  **- Under Properties, change Local copy to false for both**
- Since most of the Cosmoteer namespace is private, you need an assembly publicizer. I'm using [kraf's Publicizer](https://github.com/krafs/Publicizer).  
  kraf's Publicizer specific:  
  - In your .csproj add an ItemGroup with Publicize Include for Cosmoteer and HalflingCore  
  - In your .csproj add an ItemGroup with Compile Remove pointing to the Publicizer Package. Example:  
    ```C:\Users\user\.nuget\packages\krafs.publicizer\2.2.1\contentfiles\cs\any\Publicizer\IgnoresAccessChecksToAttribute.cs"``` 
- Add this to your .cs at the top:  
```csharp
[assembly: IgnoresAccessChecksTo("Cosmoteer")]

namespace System.Runtime.CompilerServices
{
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = true)]
    public class IgnoresAccessChecksToAttribute : Attribute
    {
        public IgnoresAccessChecksToAttribute(string assemblyName)
        {
            AssemblyName = assemblyName;
        }

        public string AssemblyName { get; }
    }
}
```

You can also look at the [Source Code](https://github.com/C0dingschmuser/EML_TestMod) of the EML Test Mod to get a better understanding on how to set your project up

### --- Troubleshooting ---

If you get the Error ```EML_Helper.dll not found``` check the eml_config.ini in your Cosmoteer installation directory. It **must** contain the path to the directory containing EML_Helper.dll and EML_Helper.runtimeconfig.json

### --- License ---
[Released under MIT License](https://github.com/C0dingschmuser/EnhancedModLoader/blob/master/LICENSE.txt)