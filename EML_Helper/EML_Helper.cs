using System.Runtime.InteropServices;
using System.Reflection;
using Cosmoteer.Gui;
using Halfling;
using Cosmoteer;
using System.Runtime.CompilerServices;
using Halfling.Application;
using static System.Net.Mime.MediaTypeNames;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Window;
using Application = System.Windows.Forms.Application;
using System.Numerics;
using Cosmoteer.Mods;
using Halfling.IO;
using Microsoft.VisualBasic;
using System.IO;

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

namespace EML_Helper
{
    public class Main
    {
        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern int MessageBox(int hWnd, string text, string caption, uint type);

        public static List<string> modDllPaths = new List<string>();
        public static bool modsLoaded = false;

        [UnmanagedCallersOnly]
        public static void InitializePatches()
        {
            Halfling.App.Director.FrameEnded += Worker;
        }

        public static void Worker(object? sender, EventArgs e)
        {
            //Called after each frame

            IAppState? currentState = App.Director.States.OfType<IAppState>().FirstOrDefault();

            if (currentState != null)
            {
                if(currentState.GetType() == typeof(TitleScreen) && !modsLoaded)
                {
                    LoadMods();
                }
            }
        }

        public static List<Halfling.IO.AbsolutePath> GetAllModPaths()
        {
            List<Halfling.IO.AbsolutePath> modPaths = new List<Halfling.IO.AbsolutePath>();

            if (Directory.Exists(Paths.BuiltInModsFolder) && Paths.BuiltInModsFolder != Paths.UserModsFolder)
            {
                string[] directories = Directory.GetDirectories((string?)Paths.BuiltInModsFolder);
                foreach (string text in directories)
                {
                    modPaths.Add((Halfling.IO.AbsolutePath)text);
                }
            }

            if (Directory.Exists(Paths.UserModsFolder))
            {
                string[] directories = Directory.GetDirectories((string?)Paths.UserModsFolder);
                foreach (string text2 in directories)
                {
                    modPaths.Add((Halfling.IO.AbsolutePath)text2);
                }
            }

            string exeDir = Application.ExecutablePath;
            //only get directory
            exeDir = exeDir.Substring(0, exeDir.LastIndexOf('\\') + 1);
            //go up 3 directories
            int lastIndex = exeDir.LastIndexOf('\\');
            if (lastIndex != -1)
            {
                for (int i = 0; i < 3; i++)
                {
                    lastIndex = exeDir.LastIndexOf('\\', lastIndex - 1);
                    if (lastIndex == -1)
                        break;
                }
            }
            string workshopDir = lastIndex != -1 ? exeDir.Substring(0, lastIndex) : exeDir;

            workshopDir += "\\workshop\\content\\799600\\";

            string[] wdirectories = Directory.GetDirectories(workshopDir);
            foreach(string folder in wdirectories)
            {
                modPaths.Add((Halfling.IO.AbsolutePath)folder);
            }

            return modPaths;
        }

        public static void GetFolderFiles(Halfling.IO.AbsolutePath folder)
        {
            string[] files =
                        Directory.GetFiles(folder.ToString() + "\\", "*.dll", SearchOption.AllDirectories);
            foreach (string file in files)
            {
                if (Settings.EnabledMods.Contains(folder))
                {
                    if (folder.ToString().Length > 0)
                    {
                        bool contains = false;
                        foreach (string entry in Main.modDllPaths)
                        {
                            if (entry.Equals(file))
                            {
                                contains = true;
                            }
                        }

                        if (!contains && !file.Contains("EML_Helper.dll") && !file.Contains("AVRT.dll"))
                        {
                            Main.modDllPaths.Add(file);
                        }
                    }
                }
            }
        }

        public static void LoadMods()
        {
            modsLoaded = true;

            foreach(Halfling.IO.AbsolutePath folder in GetAllModPaths())
            {
                GetFolderFiles(folder);
            }

            //Write enabled mods to file

            string exePath = Application.ExecutablePath;
            exePath = exePath.Substring(0, exePath.LastIndexOf('\\') + 1);

            string modPath = exePath + "eml_mods.ini";
            string lockPath = modPath + ".lock";

            if (File.Exists(modPath))
            {
                File.Delete(modPath);
            }

            if (Main.modDllPaths.Count > 0)
            {
                using StreamWriter file = new(modPath);

                foreach (string mod in Main.modDllPaths)
                {
                    file.WriteLine(mod);
                }
            }

            File.WriteAllText(lockPath, "done");

            Halfling.App.Director.FrameEnded -= Worker;
        }
    }
}