using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace CAN_TOOLS
{
    internal static class Program
    {
        // 添加Windows API声明
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool SetDllDirectory(string lpPathName);

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // 设置DLL搜索路径 - 在初始化应用程序之前
            try
            {
                string dllPath = Path.Combine(Application.StartupPath, "dll");
                SetDllDirectory(dllPath);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"设置DLL路径失败: {ex.Message}", "警告",
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }

            // To customize application configuration such as set high DPI settings or default font,
            // see https://aka.ms/applicationconfiguration.
            ApplicationConfiguration.Initialize();
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}
