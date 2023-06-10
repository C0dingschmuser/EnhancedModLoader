# EnhancedModLoader for Cosmoteer
### Automatically loads c# mod dlls from Steam Workshop Mods - Integrates with ingame mod manager.

| [Installation](#----installation----) - [How it works](#----how-it-works----) - [Developing C# Mods](#----developing-c-mods----) - [Troubleshooting](#----troubleshooting----) - [License](#----license----)|
:----------------------------------------------------------: |

### --- Installation ---
1. Subscribe to this Mod on the [Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=2937901869)  
2. Navigate to your Cosmoteer Workshop directory.  
   Usually it's ```C:\Program Files (x86)\Steam\steamapps\workshop\content\799600\2937901869\```  
   Note: ```799600 is the ID for Cosmoteer, 2937901869 is the ID for this Mod```
3. Run ```Installer.bat``` or install it manually by copying AVRT.dll from the Mod Folder to your Cosmoteer "Bin" path. This is the path where the Cosmoteer.exe lies.  
   Example: ```C:\Program Files (x86)\Steam\steamapps\common\Cosmoteer\Bin\```
4. You're good to go! Now you can just subscribe to any Workshop EML Mod and it will load automatically  
(if it's enabled in the ingame mod manager).  
5. (Optional) You can also drop c# mod dlls into the ```Bin\EML_Mods``` folder to load them
6. (Optional) If you want to test if everything works you can subscribe to the [EML Test Mod](https://steamcommunity.com/sharedfiles/filedetails/?id=2937811110).
   This simple Mod will show a test window when you enter a new game.

### --- How it works ---

AVRT.dll is a Library that Cosmoteer tries to load from it's local path that does not exist. Conveniently the local path is the first one that is being searched for this dll before searching in windows system folders and eventually loading it from there. I'm utilizing this to my advantage by copying the functionality of the original AVRT.dll + adding my own. This is called [Dll Hijacking](https://book.hacktricks.xyz/windows-hardening/windows-local-privilege-escalation/dll-hijacking).

I'm using a slightly modified version of [StackOverflowExcept1on's .net core injector](https://github.com/StackOverflowExcept1on/net-core-injector) to manually load a c# helper dll, EML_Helper.dll, to get all current enabled mods (that contain dlls) and pass them back to the original c++ dll which then loads them.

### --- Troubleshooting ---

If you get the Error ```EML_Helper.dll not found``` check the ```eml_config.ini``` in your Cosmoteer installation directory. It must contain the path to the directory containing EML_Helper.dll and EML_Helper.runtimeconfig.json

If your game crashes check the ```eml_log.txt``` file in the bin directory and make sure it's not related to a mod you subscribed to. If there is no log file or if you are sure that the crash is not related to a specific mod but the Modloader itself repeat the Installation process (overwrite existing AVRT.dll in bin folder) and try again.

### --- Developing C# Mods ---

If you want to make your own c# mod dll, this will get you started:

**IMPORTANT INFORMATION: Cosmoteer uses .NET 7. Because of this, you cannot use Harmony since it does not Support .NET 7(yet).**  
There is an [alpha version of Harmony](https://github.com/pardeike/Harmony/tree/feature/monomod-core) now that supports .NET 7 but you need to compile it yourself and i haven't tested it yet.

- Use the [.NET 7 Version of EML](https://github.com/C0dingschmuser/EnhancedModLoader/releases) / Workshop version
- Use Visual Studio C# Class Library for .NET or .NET Standard Preset
- Use target SDK 7.0.200 (Cosmoteer & EML both use this version), specify this in a global.json in your Project Directory. [See more](https://learn.microsoft.com/en-us/dotnet/core/tools/global-json)
- Use Runtime Framework Version 7.0.3
- Change your .csproj TargetFramework to
```csproj
<TargetFramework>net7.0-windows</TargetFramework>
```
- Allow Unsafe Blocks
- Set GenerateRuntimeConfigurationFiles to true

Important:

- Use an unique name for your Mod and ship your ```.runtimeconfig.json``` together with your dll (they must be in the same folder)
- Entry Point namespace **MUST** have the same name as the dll file (but without the .dll)
- Entry Point class **MUST** be named Main
- Entry Point method **MUST** be static, named ```InitializePatches``` and have the ```[UnmanagedCallersOnly]``` attribute
- Add Assembly References for Cosmoteer.dll and HalflingCore.dll from your Cosmoteer Bin Path  
  **- Under Properties, change Local copy to false for both**
- Since most of the Cosmoteer namespace is private, you need an assembly publicizer. I'm using [kraf's Publicizer](https://github.com/krafs/Publicizer).  
  kraf's Publicizer specific .csproj settings: (replace user with your username)
```csproj
<ItemGroup>
  <Compile Remove="C:\Users\user\.nuget\packages\krafs.publicizer\2.2.1\contentfiles\cs\any\Publicizer\IgnoresAccessChecksToAttribute.cs" />
</ItemGroup>
<ItemGroup>
  <Publicize Include="Cosmoteer" IncludeCompilerGeneratedMembers="false" />
  <Publicize Include="HalflingCore" IncludeCompilerGeneratedMembers="false" />
</ItemGroup>
```  
- Add this to your .cs at the top:  
```csharp
[assembly: IgnoresAccessChecksTo("Cosmoteer")]
[assembly: IgnoresAccessChecksTo("HalflingCore")]

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

Since we cannot use Harmony, there are some workarounds required to mod Cosmoteer.  
Here's [a simple example](https://github.com/C0dingschmuser/EML_TestMod) on how to get a simple loop, keyboard detection and custom window up and running.  
For more advanced use, see source of my [Weapon Projectile Spawner Mod](https://github.com/C0dingschmuser/ProjectileSpawner)

### --- License ---
[Released under MIT License](https://github.com/C0dingschmuser/EnhancedModLoader/blob/master/LICENSE.txt)
