using HarmonyLib;
using System.Reflection;
using System.Runtime.InteropServices;
using Cosmoteer.Gui;
using Halfling;
using Cosmoteer;
using Cosmoteer.Simulation;
using Cosmoteer.Bullets;
using Cosmoteer.Ships.Parts.Weapons;
using Halfling.Geometry;
using Halfling.Timing;
using Cosmoteer.Data;
using Cosmoteer.Input;
using Halfling.Input;
using static Cosmoteer.Input.Inputs;
using System.Runtime.CompilerServices;
using Halfling.Serialization.Generic;
using Cosmoteer.Game;
using Microsoft.VisualBasic;
using Cosmoteer.Mods;
using Halfling.IO;

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

        private static Harmony? harmony;
        public static List<string> modDllPaths = new List<string>();
        public static bool modsLoaded = false;

        [UnmanagedCallersOnly]
        public static void InitializePatches()
        {
            harmony = new Harmony("com.emlhelper.patch");

            Halfling.IO.AbsolutePath userModsPath = Paths.UserModsFolder;
            string[] tmpDirs = Directory.GetDirectories(Paths.UserModsFolder);
            //Main.userModsPath = userModsPath.ToString();

            harmony.PatchAll(typeof(Main).Assembly);
        }

        public static void MsgBox(string text, string caption)
        {
            MessageBox(0, text, caption, 0);
        }

    }

    [HarmonyPatch]
    public class ModEnabledPatch
    {
        static MethodBase TargetMethod()
        {
            Type[] types = new Type[1];
            types[0] = typeof(Halfling.Application.IAppState); // AccessTools.TypeByName("Halfling.Application.IAppState");

            return AccessTools.Method(typeof(Halfling.Application.Director), "SetState", types);
        }

        [HarmonyPostfix]
        private static void Postfix(Halfling.Application.IAppState state, Cosmoteer.Gui.ModsDialog __instance)
        {
            if(state.GetType() != typeof(TitleScreen) || Main.modsLoaded)
            {
                return;
            }

            //Check which dll mods are enabled

            foreach (var (folder, installSource) in Cosmoteer.Mods.ModInfo.GetAllModFolders())
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
                            foreach(string entry in Main.modDllPaths)
                            {
                                if(entry.Equals(file))
                                {
                                    contains = true;
                                }
                            }

                            if(!contains && !file.Contains("EML_Helper.dll"))
                            {
                                Main.modDllPaths.Add(file);
                            }
                        }
                    }
                }
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

                File.WriteAllText(lockPath, "done");
            }
        }
    }
}