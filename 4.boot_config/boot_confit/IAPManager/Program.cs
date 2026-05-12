using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using IAPManager;
using System.Reflection.Metadata;

namespace IAPManager
{
    public partial class MainForm : Form
    {
        private const string BATCH_FILE = "AfterBuildHandler.bat";

        // 动态路径和项目信息
        private string projectPath = "";
        private string projectName = "2xxB-LLC"; // 默认项目名称
        private string chipType = "5800"; // 默认芯片类型

        private RadioButton rbOnlineMode;
        private RadioButton rbOfflineMode;
        private CheckBox chkReplaceIapFiles;
        private Button btnSelectProject;
        private TextBox txtProjectPath;
        private Label lblProjectPath;
        private TextBox txtProjectName;
        private Label lblProjectName;
        private ComboBox cmbChipType; // 新增：芯片类型选择
        private Label lblChipType;    // 新增：芯片类型标签
        private Button btnUserConfig;
        private Button btnCompile;
        private TextBox txtLog;
        private Button btnQuickFix;
        private Button btnCreateBootLoaderFiles; // 新增按钮
        private Button btnCreatePowerShellScript; // 新增按钮
        private GroupBox gbProjectSettings;
        private GroupBox gbMode;
        private GroupBox gbActions;
        private Label lblStatus;

        public MainForm()
        {
            InitializeComponent();
            LoadCurrentConfiguration();
        }

        private void InitializeComponent()
        {
            gbProjectSettings = new GroupBox();
            lblProjectPath = new Label();
            btnSelectProject = new Button();
            txtProjectPath = new TextBox();
            lblProjectName = new Label();
            txtProjectName = new TextBox();
            lblChipType = new Label();
            cmbChipType = new ComboBox();
            gbMode = new GroupBox();
            rbOnlineMode = new RadioButton();
            chkReplaceIapFiles = new CheckBox();
            rbOfflineMode = new RadioButton();
            gbActions = new GroupBox();
            btnUserConfig = new Button();
            btnQuickFix = new Button();
            btnCreateBootLoaderFiles = new Button();
            btnCreatePowerShellScript = new Button();
            btnCompile = new Button();
            lblStatus = new Label();
            txtLog = new TextBox();
            gbProjectSettings.SuspendLayout();
            gbMode.SuspendLayout();
            gbActions.SuspendLayout();
            SuspendLayout();
            // 
            // gbProjectSettings
            // 
            gbProjectSettings.Controls.Add(lblProjectPath);
            gbProjectSettings.Controls.Add(btnSelectProject);
            gbProjectSettings.Controls.Add(txtProjectPath);
            gbProjectSettings.Controls.Add(lblProjectName);
            gbProjectSettings.Controls.Add(txtProjectName);
            gbProjectSettings.Controls.Add(lblChipType);
            gbProjectSettings.Controls.Add(cmbChipType);
            gbProjectSettings.Location = new Point(20, 20);
            gbProjectSettings.Name = "gbProjectSettings";
            gbProjectSettings.Size = new Size(740, 110);
            gbProjectSettings.TabIndex = 0;
            gbProjectSettings.TabStop = false;
            gbProjectSettings.Text = "项目设置";
            // 
            // lblProjectPath
            // 
            lblProjectPath.Location = new Point(20, 25);
            lblProjectPath.Name = "lblProjectPath";
            lblProjectPath.Size = new Size(80, 23);
            lblProjectPath.TabIndex = 0;
            lblProjectPath.Text = "工程路径:";
            lblProjectPath.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // btnSelectProject
            // 
            btnSelectProject.Location = new Point(650, 25);
            btnSelectProject.Name = "btnSelectProject";
            btnSelectProject.Size = new Size(75, 23);
            btnSelectProject.TabIndex = 2;
            btnSelectProject.Text = "选择文件夹";
            btnSelectProject.UseVisualStyleBackColor = true;
            btnSelectProject.Click += BtnSelectProject_Click;
            // 
            // txtProjectPath
            // 
            txtProjectPath.Location = new Point(100, 25);
            txtProjectPath.Name = "txtProjectPath";
            txtProjectPath.ReadOnly = true;
            txtProjectPath.Size = new Size(540, 23);
            txtProjectPath.TabIndex = 1;
            txtProjectPath.Text = "请选择工程文件夹...";
            // 
            // lblProjectName
            // 
            lblProjectName.Location = new Point(20, 55);
            lblProjectName.Name = "lblProjectName";
            lblProjectName.Size = new Size(80, 23);
            lblProjectName.TabIndex = 3;
            lblProjectName.Text = "项目名称:";
            lblProjectName.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // txtProjectName
            // 
            txtProjectName.Location = new Point(100, 55);
            txtProjectName.Name = "txtProjectName";
            txtProjectName.Size = new Size(200, 23);
            txtProjectName.TabIndex = 4;
            txtProjectName.Text = "210B-LLC";
            txtProjectName.TextChanged += TxtProjectName_TextChanged;
            // 
            // lblChipType
            // 
            lblChipType.Location = new Point(320, 55);
            lblChipType.Name = "lblChipType";
            lblChipType.Size = new Size(80, 23);
            lblChipType.TabIndex = 5;
            lblChipType.Text = "芯片类型:";
            lblChipType.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // cmbChipType
            // 
            cmbChipType.DropDownStyle = ComboBoxStyle.DropDownList;
            cmbChipType.Items.AddRange(new object[] { "5800", "5300" });
            cmbChipType.Location = new Point(400, 55);
            cmbChipType.Name = "cmbChipType";
            cmbChipType.Size = new Size(80, 25);
            cmbChipType.TabIndex = 6;
            cmbChipType.SelectedIndexChanged += CmbChipType_SelectedIndexChanged;
            // 
            // gbMode
            // 
            gbMode.Controls.Add(rbOnlineMode);
            gbMode.Controls.Add(chkReplaceIapFiles);
            gbMode.Controls.Add(rbOfflineMode);
            gbMode.Location = new Point(20, 150);
            gbMode.Name = "gbMode";
            gbMode.Size = new Size(300, 130);
            gbMode.TabIndex = 1;
            gbMode.TabStop = false;
            gbMode.Text = "升级模式选择";
            // 
            // rbOnlineMode
            // 
            rbOnlineMode.Location = new Point(20, 25);
            rbOnlineMode.Name = "rbOnlineMode";
            rbOnlineMode.Size = new Size(200, 25);
            rbOnlineMode.TabIndex = 0;
            rbOnlineMode.Text = "在线升级模式 (IAP)";
            rbOnlineMode.CheckedChanged += RbMode_CheckedChanged;
            // 
            // chkReplaceIapFiles
            // 
            chkReplaceIapFiles.Enabled = false;
            chkReplaceIapFiles.Location = new Point(40, 55);
            chkReplaceIapFiles.Name = "chkReplaceIapFiles";
            chkReplaceIapFiles.Size = new Size(200, 25);
            chkReplaceIapFiles.TabIndex = 1;
            chkReplaceIapFiles.Text = "替换iap文件 (复制jlink文件)";
            chkReplaceIapFiles.CheckedChanged += ChkReplaceIapFiles_CheckedChanged;
            // 
            // rbOfflineMode
            // 
            rbOfflineMode.Location = new Point(20, 85);
            rbOfflineMode.Name = "rbOfflineMode";
            rbOfflineMode.Size = new Size(200, 25);
            rbOfflineMode.TabIndex = 2;
            rbOfflineMode.Text = "非在线升级模式 (直接烧录)";
            rbOfflineMode.CheckedChanged += RbMode_CheckedChanged;
            // 
            // gbActions
            // 
            gbActions.Controls.Add(btnUserConfig);
            gbActions.Controls.Add(btnQuickFix);
            gbActions.Controls.Add(btnCreateBootLoaderFiles);
            gbActions.Controls.Add(btnCreatePowerShellScript);
            gbActions.Controls.Add(btnCompile);
            gbActions.Location = new Point(340, 150);
            gbActions.Name = "gbActions";
            gbActions.Size = new Size(420, 160);
            gbActions.TabIndex = 2;
            gbActions.TabStop = false;
            gbActions.Text = "操作";
            // 
            // btnUserConfig
            // 
            btnUserConfig.Location = new Point(20, 29);
            btnUserConfig.Name = "btnUserConfig";
            btnUserConfig.Size = new Size(65, 30);
            btnUserConfig.TabIndex = 1;
            btnUserConfig.Text = "应用配置";
            btnUserConfig.Click += BtnUserConfig_Click;
            // 
            // btnQuickFix
            // 
            btnQuickFix.BackColor = Color.Orange;
            btnQuickFix.ForeColor = SystemColors.WindowText;
            btnQuickFix.Location = new Point(102, 29);
            btnQuickFix.Name = "btnQuickFix";
            btnQuickFix.Size = new Size(100, 30);
            btnQuickFix.TabIndex = 2;
            btnQuickFix.Text = "初始化sct文件";
            btnQuickFix.UseVisualStyleBackColor = false;
            btnQuickFix.Click += BtnQuickFix_Click;
            // 
            // btnCreateBootLoaderFiles
            // 
            btnCreateBootLoaderFiles.BackColor = Color.LightBlue;
            btnCreateBootLoaderFiles.Location = new Point(220, 29);
            btnCreateBootLoaderFiles.Name = "btnCreateBootLoaderFiles";
            btnCreateBootLoaderFiles.Size = new Size(120, 30);
            btnCreateBootLoaderFiles.TabIndex = 3;
            btnCreateBootLoaderFiles.Text = "创建BootLoader文件";
            btnCreateBootLoaderFiles.UseVisualStyleBackColor = false;
            btnCreateBootLoaderFiles.Click += BtnCreateBootLoaderFiles_Click;
            // 
            // btnCreatePowerShellScript
            // 
            btnCreatePowerShellScript.BackColor = Color.LightGreen;
            btnCreatePowerShellScript.Location = new Point(350, 29);
            btnCreatePowerShellScript.Name = "btnCreatePowerShellScript";
            btnCreatePowerShellScript.Size = new Size(60, 30);
            btnCreatePowerShellScript.TabIndex = 4;
            btnCreatePowerShellScript.Text = "创建PS脚本";
            btnCreatePowerShellScript.UseVisualStyleBackColor = false;
            btnCreatePowerShellScript.Click += BtnCreatePowerShellScript_Click;
            // 
            // btnCompile
            // 
            btnCompile.Enabled = false;
            btnCompile.Location = new Point(10, 95);
            btnCompile.Name = "btnCompile";
            btnCompile.Size = new Size(405, 30);
            btnCompile.TabIndex = 5;
            btnCompile.Text = "编译合并";
            btnCompile.Click += BtnCompile_Click;
            // 
            // lblStatus
            // 
            lblStatus.ForeColor = Color.Blue;
            lblStatus.Location = new Point(20, 330);
            lblStatus.Name = "lblStatus";
            lblStatus.Size = new Size(500, 25);
            lblStatus.TabIndex = 3;
            lblStatus.Text = "状态: 未选择工程";
            // 
            // txtLog
            // 
            txtLog.Font = new Font("Consolas", 9F);
            txtLog.Location = new Point(20, 360);
            txtLog.Multiline = true;
            txtLog.Name = "txtLog";
            txtLog.ReadOnly = true;
            txtLog.ScrollBars = ScrollBars.Vertical;
            txtLog.Size = new Size(740, 180);
            txtLog.TabIndex = 4;
            // 
            // MainForm
            // 
            ClientSize = new Size(784, 561);
            Controls.Add(gbProjectSettings);
            Controls.Add(gbMode);
            Controls.Add(gbActions);
            Controls.Add(lblStatus);
            Controls.Add(txtLog);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            MaximizeBox = false;
            Name = "MainForm";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "ZHLD_IAP_CONFIG v3.0";
            gbProjectSettings.ResumeLayout(false);
            gbProjectSettings.PerformLayout();
            gbMode.ResumeLayout(false);
            gbActions.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }

        // 新增：芯片类型选择事件处理
        private void CmbChipType_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cmbChipType.SelectedItem != null)
            {
                chipType = cmbChipType.SelectedItem.ToString();
                LogMessage($"芯片类型已切换为: {chipType}");

                // 更新按钮文本以反映当前芯片类型
                btnCreateBootLoaderFiles.Text = $"创建BootLoader文件({chipType})";
            }
        }

        // 获取当前芯片类型对应的BootLoader数据
        private byte[] GetCurrentBootLoaderBinData()
        {
            switch (chipType)
            {
                case "5300":
                    return MyData.BOOTLOADER_5300_BIN_DATA;
                case "5800":
                default:
                    return MyData.BOOTLOADER_5800_BIN_DATA;
            }
        }

        private byte[] GetCurrentBootLoaderHexData()
        {
            switch (chipType)
            {
                case "5300":
                    return MyData.BOOTLOADER_5300_HEX_DATA;
                case "5800":
                default:
                    return MyData.BOOTLOADER_5800_HEX_DATA;
            }
        }
        // 需要在MainForm类中添加或修改的方法

        // 1. 修改 createNewJsonConfig 方法，支持不同芯片类型的配置
        private void CreateNewJsonConfig(string jsonFile, string binName, string outName, string chipType)
        {
            string bootFileName, appFileName, outFileName;
            int bootMaxSize, argBaseAddr, appBaseAddr, appMaxSize;

            if (chipType == "5300")
            {
                bootFileName = $"{projectName}/BootLoader-5300.bin";
                appFileName = $"{projectName}/{binName}.bin";
                outFileName = $"{projectName}/{outName}.bin";
                bootMaxSize = 30720;   // 30KB
                argBaseAddr = 30240;   // 0x7620
                appBaseAddr = 30720;   // 0x7800
                appMaxSize = 46080;    // 45KB
            }
            else // 5800 默认
            {
                bootFileName = $"{projectName}/BootLoader-5800.bin";
                appFileName = $"{projectName}/{binName}.bin";
                outFileName = $"{projectName}/{outName}.bin";
                bootMaxSize = 32768;   // 32KB
                argBaseAddr = 28672;   // 0x7000
                appBaseAddr = 32768;   // 0x8000
                appMaxSize = 229376;   // 224KB
            }

            var jsonContent = @"{
  ""bootFile"": """ + bootFileName + @""",
  ""appFile"": """ + appFileName + @""",
  ""outFile"": """ + outFileName + @""",
  ""bootMaxSize"": " + bootMaxSize + @",
  ""argBaseAddr"": " + argBaseAddr + @",
  ""appBaseAddr"": " + appBaseAddr + @",
  ""appMaxSize"": " + appMaxSize + @"
}";

            File.WriteAllText(jsonFile, jsonContent);
            LogMessage($"创建新的{chipType}芯片JSON配置文件: {jsonFile}");
        }

        // 2. 修改 createNewHexJsonConfig 方法，支持不同芯片类型的配置
        private void CreateNewHexJsonConfig(string jsonFile, string hexName, string outName, string chipType)
        {
            string bootFileName, appFileName, outFileName;
            int bootMaxSize, argBaseAddr, appBaseAddr, appMaxSize, flashBaseAddr;

            if (chipType == "5300")
            {
                bootFileName = $"{projectName}/BootLoader-5300.hex";
                appFileName = $"{projectName}/{hexName}.hex";
                outFileName = $"{projectName}/{outName}.hex";
                bootMaxSize = 30720;      // 30KB
                argBaseAddr = 30240;      // 0x7620
                appBaseAddr = 30720;      // 0x7800
                appMaxSize = 46080;       // 45KB
                flashBaseAddr = 134217728; // 0x08000000
            }
            else // 5800 默认
            {
                bootFileName = $"{projectName}/BootLoader-5800.hex";
                appFileName = $"{projectName}/{hexName}.hex";
                outFileName = $"{projectName}/{outName}.hex";
                bootMaxSize = 32768;      // 32KB
                argBaseAddr = 28672;      // 0x7000
                appBaseAddr = 32768;      // 0x8000
                appMaxSize = 229376;      // 224KB
                flashBaseAddr = 134217728; // 0x08000000
            }

            var jsonContent = @"{
  ""bootFile"": """ + bootFileName + @""",
  ""appFile"": """ + appFileName + @""",
  ""outFile"": """ + outFileName + @""",
  ""bootMaxSize"": " + bootMaxSize + @",
  ""argBaseAddr"": " + argBaseAddr + @",
  ""appBaseAddr"": " + appBaseAddr + @",
  ""appMaxSize"": " + appMaxSize + @",
  ""flashBaseAddr"": " + flashBaseAddr + @"
}";

            File.WriteAllText(jsonFile, jsonContent);
            LogMessage($"创建新的{chipType}芯片HEX JSON配置文件: {jsonFile}");
        }

        // 3. 修改 createNewIapToolJsonConfig 方法
        private void CreateNewIapToolJsonConfig(string jsonFile, string binName, string chipType)
        {
            string appFileName = $"{projectName}/{binName}.bin";
            int flashBaseAddr, flashMaxSize, appBaseAddr, appMaxSize, argBaseAddr, readWriteSize;

            if (chipType == "5300")
            {
                // 5300芯片的绝对地址配置
                flashBaseAddr = 134217728;   // 0x08000000
                flashMaxSize = 76800;        // 0x12C00 (整个flash大小)
                appBaseAddr = 134248448;     // 0x08007800 (flash基址 + 30720)
                appMaxSize = 46080;          // 0xB400 (45KB)
                argBaseAddr = 134247968;     // 0x08007620 (flash基址 + 30240)
                readWriteSize = 240;         // 5300芯片使用240 (可能需要根据实际情况调整)
            }
            else // 5800 默认
            {
                // 5800芯片的绝对地址配置
                flashBaseAddr = 134217728;   // 0x08000000
                flashMaxSize = 262144;       // 0x40000 (256KB)
                appBaseAddr = 134250496;     // 0x08008000 (flash基址 + 32768)
                appMaxSize = 229376;         // 0x38000 (224KB)
                argBaseAddr = 134246400;     // 0x08007000 (flash基址 + 28672)
                readWriteSize = 128;         // 5800芯片使用128 (根据你的实际配置)
            }

            var jsonContent = $@"{{
  ""baudrate"": 115200,
  ""modbusAddr"": 2,
  ""appFile"": ""{appFileName}"",
  ""flashBaseAddr"": {flashBaseAddr},
  ""flashMaxSize"": {flashMaxSize},
  ""appBaseAddr"": {appBaseAddr},
  ""appMaxSize"": {appMaxSize},
  ""argBaseAddr"": {argBaseAddr},
  ""readWriteSize"": {readWriteSize}
}}";

            File.WriteAllText(jsonFile, jsonContent);
            LogMessage($"创建新的{chipType}芯片IapTool BIN JSON配置文件: {jsonFile}");
        }

        // 4. 修改 createNewIapToolHexJsonConfig 方法
        private void CreateNewIapToolHexJsonConfig(string jsonFile, string hexName, string chipType)
        {
            string appFileName = $"{projectName}/{hexName}.hex";
            int flashBaseAddr, flashMaxSize, appBaseAddr, appMaxSize, argBaseAddr, readWriteSize;

            if (chipType == "5300")
            {
                // 5300芯片的绝对地址配置
                flashBaseAddr = 134217728;   // 0x08000000
                flashMaxSize = 76800;        // 0x12C00 (整个flash大小)
                appBaseAddr = 134248448;     // 0x08007800 (flash基址 + 30720)
                appMaxSize = 46080;          // 0xB400 (45KB)
                argBaseAddr = 134247968;     // 0x08007620 (flash基址 + 30240)
                readWriteSize = 240;         // 5300芯片使用240 (可能需要根据实际情况调整)
            }
            else // 5800 默认
            {
                // 5800芯片的绝对地址配置
                flashBaseAddr = 134217728;   // 0x08000000
                flashMaxSize = 262144;       // 0x40000 (256KB)
                appBaseAddr = 134250496;     // 0x08008000 (flash基址 + 32768)
                appMaxSize = 229376;         // 0x38000 (224KB)
                argBaseAddr = 134246400;     // 0x08007000 (flash基址 + 28672)
                readWriteSize = 128;         // 5800芯片使用128 (根据你的实际配置)
            }

            var jsonContent = $@"{{
  ""baudrate"": 115200,
  ""modbusAddr"": 2,
  ""appFile"": ""{appFileName}"",
  ""flashBaseAddr"": {flashBaseAddr},
  ""flashMaxSize"": {flashMaxSize},
  ""appBaseAddr"": {appBaseAddr},
  ""appMaxSize"": {appMaxSize},
  ""argBaseAddr"": {argBaseAddr},
  ""readWriteSize"": {readWriteSize}
}}";

            File.WriteAllText(jsonFile, jsonContent);
            LogMessage($"创建新的{chipType}芯片IapTool HEX JSON配置文件: {jsonFile}");
        }

        // 5. 修改 UpdateBatchFile 方法，传递芯片类型信息
        private void UpdateBatchFile(bool isOnlineMode, bool replaceIapFiles)
        {
            try
            {
                string batchPath = Path.Combine(projectPath, BATCH_FILE);
                string batchContent;

                if (isOnlineMode)
                {
                    // 根据芯片类型选择对应的模板
                    string template;
                    if (chipType == "5300")
                    {
                        template = replaceIapFiles ? MyData.ONLINE_BATCH_TEMPLATE_WITH_STEP6_5300 : MyData.ONLINE_BATCH_TEMPLATE_WITHOUT_STEP6_5300;
                        LogMessage($"使用5300芯片专用模板");
                    }
                    else // 5800
                    {
                        template = replaceIapFiles ? MyData.ONLINE_BATCH_TEMPLATE_WITH_STEP6 : MyData.ONLINE_BATCH_TEMPLATE_WITHOUT_STEP6;
                        LogMessage($"使用5800芯片专用模板");
                    }

                    // 替换项目名称
                    batchContent = template.Replace("{PROJECT_NAME}", projectName);

                    LogMessage($"在线模式 - {(replaceIapFiles ? "包含" : "不包含")}替换iap文件功能 (芯片: {chipType})");

                    // 自动创建BootLoader文件和PowerShell脚本
                    CreateEmbeddedBootLoaderFiles();
                    CreateEmbeddedPowerShellScript();
                }
                else
                {
                    batchContent = MyData.OFFLINE_BATCH_TEMPLATE;
                    LogMessage($"非在线模式 (芯片: {chipType})");
                }

                if (File.Exists(batchPath))
                {
                    string backupPath = batchPath + ".backup";
                    File.Copy(batchPath, backupPath, true);
                    LogMessage($"已备份原批处理文件到: {backupPath}");
                }

                File.WriteAllText(batchPath, batchContent, new UTF8Encoding(false));

                LogMessage($"已更新批处理文件为{(isOnlineMode ? "在线" : "非在线")}模式 (芯片: {chipType})");
                if (isOnlineMode && replaceIapFiles)
                {
                    LogMessage("批处理文件包含Step 6: 拷贝jlink文件到Execute目录");
                }
                LogMessage($"批处理文件路径: {batchPath}");
            }
            catch (Exception ex)
            {
                LogMessage($"更新批处理文件失败: {ex.Message}");
                throw;
            }
        }

        // 6. 需要在 MyData 类中添加 5300 的 PowerShell 脚本内容（如果需要特定版本）
        // 如果PowerShell脚本是通用的，可以使用相同的脚本
        private void CreateEmbeddedPowerShellScript()
        {
            try
            {
                string psScriptPath = Path.Combine(projectPath, "MergeIapFiles.ps1");

                // 检查文件是否已存在，如果不存在则创建
                if (!File.Exists(psScriptPath))
                {
                    // 使用通用的PowerShell脚本，因为它通过JSON配置获取参数
                    File.WriteAllText(psScriptPath, MyData.MERGE_IAP_FILES_PS1_CONTENT, new UTF8Encoding(false));
                    LogMessage($"自动创建PowerShell脚本: {psScriptPath}");
                }
                else
                {
                    // 检查文件大小是否匹配（简单的验证方式）
                    var fileInfo = new FileInfo(psScriptPath);
                    if (fileInfo.Length < 1000) // 如果文件太小，可能是损坏的，重新创建
                    {
                        File.WriteAllText(psScriptPath, MyData.MERGE_IAP_FILES_PS1_CONTENT, new UTF8Encoding(false));
                        LogMessage($"检测到PowerShell脚本文件异常，已重新创建: {psScriptPath}");
                    }
                    else
                    {
                        LogMessage("PowerShell脚本文件已存在，跳过创建");
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage($"自动创建PowerShell脚本时出错: {ex.Message}");
            }
        }

        // 7. 在批处理文件模板中需要调用JSON配置创建函数时传递芯片类型
        // 需要修改批处理文件模板中的函数调用，让它们能够根据芯片类型创建正确的配置

        // 9. 需要在MyData类中添加5300专用的批处理文件模板常量
        // MyData.ONLINE_BATCH_TEMPLATE_WITHOUT_STEP6_5300
        // MyData.ONLINE_BATCH_TEMPLATE_WITH_STEP6_5300
        // 这些模板中的JSON配置创建函数调用应该使用5300的参数

        // 10. 修改现有的updateJsonConfig等方法的调用，传递芯片类型
        // 需要修改批处理文件中的函数调用，或者在C#中直接创建JSON文件

        // 11. 添加一个统一的JSON配置创建方法
        private void CreateJsonConfigFile(string configType, string targetName)
        {
            try
            {
                string configDir = Path.Combine(projectPath, $@"IAP\IapFileCreator\{projectName}");
                Directory.CreateDirectory(configDir);

                if (configType.Contains("BIN"))
                {
                    string jsonPath = Path.Combine(configDir, $"{projectName}-BIN.json");
                    string outName = targetName.Replace("iap", "jlink");
                    CreateNewJsonConfig(jsonPath, targetName, outName, chipType);
                }

                if (configType.Contains("HEX"))
                {
                    string jsonPath = Path.Combine(configDir, $"{projectName}-HEX.json");
                    string outName = targetName.Replace("iap", "jlink");
                    CreateNewHexJsonConfig(jsonPath, targetName, outName, chipType);
                }

                // 同时为IapTool目录创建配置
                string iapToolDir = Path.Combine(projectPath, $@"IAP\IapTool\{projectName}");
                Directory.CreateDirectory(iapToolDir);

                if (configType.Contains("BIN"))
                {
                    string iapToolJsonPath = Path.Combine(iapToolDir, $"{projectName}-BIN.json");
                    CreateNewIapToolJsonConfig(iapToolJsonPath, targetName, chipType);
                }

                if (configType.Contains("HEX"))
                {
                    string iapToolJsonPath = Path.Combine(iapToolDir, $"{projectName}-HEX.json");
                    CreateNewIapToolHexJsonConfig(iapToolJsonPath, targetName, chipType);
                }

                LogMessage($"已创建{chipType}芯片的{configType}配置文件");
            }
            catch (Exception ex)
            {
                LogMessage($"创建{chipType}芯片JSON配置文件失败: {ex.Message}");
            }
        }

        // 12. 修改CreateEmbeddedBootLoaderFiles方法的调用，确保使用正确的芯片类型文件名
        private void CreateEmbeddedBootLoaderFiles()
        {
            try
            {
                string iapDir = Path.Combine(projectPath, $@"IAP\IapFileCreator\{projectName}");
                Directory.CreateDirectory(iapDir);

                var binData = GetCurrentBootLoaderBinData();
                var hexData = GetCurrentBootLoaderHexData();

                string binPath = Path.Combine(iapDir, $"BootLoader-{chipType}.bin");
                string hexPath = Path.Combine(iapDir, $"BootLoader-{chipType}.hex");

                // 检查文件是否已存在，如果不存在或者大小不匹配则创建
                bool needCreateBin = !File.Exists(binPath) ||
                    new FileInfo(binPath).Length != binData.Length;
                bool needCreateHex = !File.Exists(hexPath) ||
                    new FileInfo(hexPath).Length != hexData.Length;

                if (needCreateBin)
                {
                    File.WriteAllBytes(binPath, binData);
                    if (binData.Length <= 32)
                    {
                        LogMessage($"自动创建{chipType}芯片BootLoader BIN文件（占位符数据）: {binPath}");
                    }
                    else
                    {
                        LogMessage($"自动创建{chipType}芯片BootLoader BIN文件: {binPath}");
                    }
                }

                if (needCreateHex)
                {
                    File.WriteAllBytes(hexPath, hexData);
                    if (hexData.Length <= 100)
                    {
                        LogMessage($"自动创建{chipType}芯片BootLoader HEX文件（占位符数据）: {hexPath}");
                    }
                    else
                    {
                        LogMessage($"自动创建{chipType}芯片BootLoader HEX文件: {hexPath}");
                    }
                }

                if (!needCreateBin && !needCreateHex)
                {
                    LogMessage($"{chipType}芯片BootLoader文件已存在且大小匹配，跳过创建");
                }
            }
            catch (Exception ex)
            {
                LogMessage($"自动创建{chipType}芯片BootLoader文件时出错: {ex.Message}");
            }
        }

        // 13. 完整的LoadCurrentConfiguration方法修改
        private void LoadCurrentConfiguration()
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                LogMessage("请先选择工程文件夹");
                return;
            }

            try
            {
                string scatterFileName = GetScatterFileName(chipType);
                string scatterPath = Path.Combine(projectPath, scatterFileName);

                if (File.Exists(scatterPath))
                {
                    string content = File.ReadAllText(scatterPath, new UTF8Encoding(false));

                    bool isOnlineMode = false;

                    if (chipType == "5300")
                    {
                        // 5300芯片的配置检测
                        bool rom8000Active = Regex.IsMatch(content, @"^[^;/*]*#define\s+__ROM_BASE\s+0x08000000", RegexOptions.Multiline);
                        bool rom7800Active = Regex.IsMatch(content, @"^[^;/*]*#define\s+__ROM_BASE\s+0x08007800", RegexOptions.Multiline);

                        LogMessage($"5300芯片配置检测结果:");
                        LogMessage($"  - ROM_BASE 0x08000000: 激活={rom8000Active}");
                        LogMessage($"  - ROM_BASE 0x08007800: 激活={rom7800Active}");

                        if (rom7800Active)
                        {
                            isOnlineMode = true;
                            LogMessage("当前配置: 在线升级模式 (ROM_BASE=0x08007800)");
                        }
                        else if (rom8000Active)
                        {
                            isOnlineMode = false;
                            LogMessage("当前配置: 非在线升级模式 (ROM_BASE=0x08000000)");
                        }
                        else
                        {
                            LogMessage("警告: 没有检测到激活的ROM_BASE配置");
                        }
                    }
                    else // 5800芯片
                    {
                        // 5800芯片的配置检测
                        bool rom8000Active = Regex.IsMatch(content, @"^[^;/*]*#define\s+__ROM_BASE\s+0x08000000", RegexOptions.Multiline);
                        bool rom8008Active = Regex.IsMatch(content, @"^[^;/*]*#define\s+__ROM_BASE\s+0x08008000", RegexOptions.Multiline);

                        LogMessage($"5800芯片配置检测结果:");
                        LogMessage($"  - ROM_BASE 0x08000000: 激活={rom8000Active}");
                        LogMessage($"  - ROM_BASE 0x08008000: 激活={rom8008Active}");

                        if (rom8008Active)
                        {
                            isOnlineMode = true;
                            LogMessage("当前配置: 在线升级模式 (ROM_BASE=0x08008000)");
                        }
                        else if (rom8000Active)
                        {
                            isOnlineMode = false;
                            LogMessage("当前配置: 非在线升级模式 (ROM_BASE=0x08000000)");
                        }
                        else
                        {
                            LogMessage("警告: 没有检测到激活的ROM_BASE配置");
                        }
                    }

                    // 更新UI状态
                    if (isOnlineMode)
                    {
                        rbOnlineMode.Checked = true;
                        chkReplaceIapFiles.Enabled = true;
                        btnCompile.Enabled = true;
                    }
                    else
                    {
                        rbOfflineMode.Checked = true;
                        chkReplaceIapFiles.Enabled = false;
                        chkReplaceIapFiles.Checked = false;
                        btnCompile.Enabled = false;
                    }

                    lblStatus.Text = $"状态: 已加载当前配置 ({chipType}芯片)";
                    lblStatus.ForeColor = System.Drawing.Color.Green;
                }
                else
                {
                    LogMessage($"错误: 找不到{chipType}芯片scatter文件: {scatterPath}");

                    // 检查是否存在其他芯片类型的scatter文件
                    string otherChipType = chipType == "5300" ? "5800" : "5300";
                    string otherScatterPath = Path.Combine(projectPath, GetScatterFileName(otherChipType));
                    if (File.Exists(otherScatterPath))
                    {
                        LogMessage($"💡 但找到了{otherChipType}芯片的scatter文件，建议切换芯片类型");
                        lblStatus.Text = $"状态: 找到{otherChipType}芯片文件，请切换芯片类型";
                        lblStatus.ForeColor = System.Drawing.Color.Orange;
                    }
                    else
                    {
                        lblStatus.Text = "状态: 文件不存在";
                        lblStatus.ForeColor = System.Drawing.Color.Red;
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage($"加载配置时出错: {ex.Message}");
                lblStatus.Text = "状态: 加载失败";
                lblStatus.ForeColor = System.Drawing.Color.Red;
            }
        }

        // ============================================================================
        // 需要在MyData类中添加的常量（5300专用的批处理文件模板）
        // ============================================================================

        /*
        需要在MyData类中添加以下常量：

        public const string ONLINE_BATCH_TEMPLATE_WITHOUT_STEP6_5300 = @"
        // 基本上复制ONLINE_BATCH_TEMPLATE_WITHOUT_STEP6，但修改以下配置：
        // - BootLoader-5800 改为 BootLoader-5300
        // - 所有JSON配置创建函数中的参数改为5300的值：
        //   bootMaxSize: 30720
        //   argBaseAddr: 30240  
        //   appBaseAddr: 30720
        //   appMaxSize: 46080
        ";

        public const string ONLINE_BATCH_TEMPLATE_WITH_STEP6_5300 = @"
        // 基本上复制ONLINE_BATCH_TEMPLATE_WITH_STEP6，但修改以下配置：
        // - BootLoader-5800 改为 BootLoader-5300
        // - 所有JSON配置创建函数中的参数改为5300的值：
        //   bootMaxSize: 30720
        //   argBaseAddr: 30240  
        //   appBaseAddr: 30720
        //   appMaxSize: 46080
        ";

        或者，更好的方法是修改现有的批处理文件模板，让JSON配置创建函数能够
        动态获取芯片类型信息，但这在批处理文件中比较困难。

        建议的实现方式：
        1. 在C#程序中直接创建JSON配置文件，而不是依赖批处理文件
        2. 修改批处理文件，让它只负责编译和合并，JSON配置由C#程序预先创建
        */

        // 14. 建议的改进：在应用配置时直接创建JSON配置文件
        private void BtnUserConfig_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                MessageBox.Show("请先选择工程文件夹！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (string.IsNullOrEmpty(projectName))
            {
                MessageBox.Show("请输入项目名称！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (!rbOnlineMode.Checked && !rbOfflineMode.Checked)
            {
                MessageBox.Show("请先选择升级模式！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            try
            {
                LogMessage("=== 开始应用配置 ===");
                LogMessage($"工程路径: {projectPath}");
                LogMessage($"项目名称: {projectName}");
                LogMessage($"芯片类型: {chipType}");

                string scatterFileName = GetScatterFileName(chipType);
                string scatterPath = Path.Combine(projectPath, scatterFileName);

                if (!File.Exists(scatterPath))
                {
                    LogMessage($"错误: {chipType}芯片Scatter文件不存在: {scatterPath}");

                    // 检查是否存在其他芯片类型的文件
                    string otherChipType = chipType == "5300" ? "5800" : "5300";
                    string otherScatterPath = Path.Combine(projectPath, GetScatterFileName(otherChipType));
                    if (File.Exists(otherScatterPath))
                    {
                        MessageBox.Show($"当前目录包含{otherChipType}芯片的项目文件！\n\n请：\n1. 切换芯片类型为{otherChipType}，或\n2. 选择正确的{chipType}芯片项目目录",
                            "芯片类型不匹配", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    }
                    else
                    {
                        MessageBox.Show($"找不到{chipType}芯片的Scatter文件！\n请确保选择了正确的项目目录。",
                            "文件不存在", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    return;
                }

                string backupPath = scatterPath + ".backup";
                File.Copy(scatterPath, backupPath, true);
                LogMessage($"已备份原文件到: {backupPath}");

                string content = File.ReadAllText(scatterPath, new UTF8Encoding(false));

                if (!ValidateScatterFileFormat(content))
                {
                    LogMessage("错误: Scatter文件格式不正确！");
                    MessageBox.Show("Scatter文件格式不正确！请检查第一行是否为ARM预处理器指令。", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // 确保基本目录结构存在
                LogMessage("检查并创建基本目录结构...");
                CreateBasicDirectories();

                if (rbOnlineMode.Checked)
                {
                    LogMessage("配置为在线升级模式...");
                    content = ConfigureOnlineMode(content);
                    btnCompile.Enabled = true;
                    UpdateBatchFile(true, chkReplaceIapFiles.Checked);

                    // 预先创建JSON配置文件模板
                    CreateDefaultJsonConfigs();
                }
                else if (rbOfflineMode.Checked)
                {
                    LogMessage("配置为非在线升级模式...");
                    content = ConfigureOfflineMode(content);
                    btnCompile.Enabled = false;
                    UpdateBatchFile(false, false);
                }

                File.WriteAllText(scatterPath, content, new UTF8Encoding(false));

                CreateIAPDirectories();

                LogMessage("=== 配置应用完成 ===");
                lblStatus.Text = $"状态: 已配置为{(rbOnlineMode.Checked ? "在线" : "非在线")}模式 ({chipType}芯片)";
                lblStatus.ForeColor = System.Drawing.Color.Green;

                MessageBox.Show($"配置已成功应用！\n芯片类型: {chipType}", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                LogMessage($"应用配置失败: {ex.Message}");
                lblStatus.Text = "状态: 配置失败";
                lblStatus.ForeColor = System.Drawing.Color.Red;
                MessageBox.Show($"配置失败: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        private void CreateDefaultJsonConfigs()
        {
            try
            {
                LogMessage($"为{chipType}芯片创建默认JSON配置文件...");

                // 创建IAP目录
                string iapCreatorDir = Path.Combine(projectPath, $@"IAP\IapFileCreator\{projectName}");
                string iapToolDir = Path.Combine(projectPath, $@"IAP\IapTool\{projectName}");
                Directory.CreateDirectory(iapCreatorDir);
                Directory.CreateDirectory(iapToolDir);

                // 创建一个默认的应用文件名
                string defaultAppName = $"{projectName}_iap";

                // 获取芯片特定的配置参数
                var chipConfig = GetChipConfig(chipType);

                // 创建IapFileCreator的JSON配置（相对地址格式）
                CreateIapFileCreatorJsonConfig(Path.Combine(iapCreatorDir, $"{projectName}-BIN.json"), chipConfig, defaultAppName, "BIN", true);
                CreateIapFileCreatorJsonConfig(Path.Combine(iapCreatorDir, $"{projectName}-HEX.json"), chipConfig, defaultAppName, "HEX", true);

                // 创建IapTool的JSON配置（绝对地址格式）
                CreateIapToolJsonConfig(Path.Combine(iapToolDir, $"{projectName}-BIN.json"), chipConfig, defaultAppName, "BIN");
                CreateIapToolJsonConfig(Path.Combine(iapToolDir, $"{projectName}-HEX.json"), chipConfig, defaultAppName, "HEX");

                LogMessage($"{chipType}芯片默认JSON配置文件创建完成");
            }
            catch (Exception ex)
            {
                LogMessage($"创建默认JSON配置文件失败: {ex.Message}");
            }
        }

        // 2. 创建IapFileCreator格式的JSON配置（相对地址）
        private void CreateIapFileCreatorJsonConfig(string jsonPath, dynamic config, string appName, string format, bool includeOutFile)
        {
            try
            {
                string bootFile = $"{projectName}/{config.bootFileName}.{format.ToLower()}";
                string appFile = $"{projectName}/{appName}.{format.ToLower()}";

                string jsonContent;
                if (format == "HEX")
                {
                    if (includeOutFile)
                    {
                        string outFile = $"{projectName}/{appName.Replace("iap", "jlink")}.{format.ToLower()}";
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""outFile"": ""{outFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize},
  ""flashBaseAddr"": {config.flashBaseAddr}
}}";
                    }
                    else
                    {
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize},
  ""flashBaseAddr"": {config.flashBaseAddr}
}}";
                    }
                }
                else // BIN
                {
                    if (includeOutFile)
                    {
                        string outFile = $"{projectName}/{appName.Replace("iap", "jlink")}.{format.ToLower()}";
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""outFile"": ""{outFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize}
}}";
                    }
                    else
                    {
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize}
}}";
                    }
                }

                File.WriteAllText(jsonPath, jsonContent);
                LogMessage($"创建{chipType}芯片IapFileCreator {format}配置: {Path.GetFileName(jsonPath)}");
            }
            catch (Exception ex)
            {
                LogMessage($"创建IapFileCreator JSON配置文件失败 {jsonPath}: {ex.Message}");
            }
        }

        // 3. 创建IapTool格式的JSON配置（绝对地址）
        private void CreateIapToolJsonConfig(string jsonPath, dynamic config, string appName, string format)
        {
            try
            {
                string appFile = $"{projectName}/{appName}.{format.ToLower()}";

                // 计算绝对地址
                int flashBaseAddr = 134217728; // 0x08000000
                int flashMaxSize, appBaseAddr, argBaseAddr, readWriteSize;

                if (chipType == "5300")
                {
                    flashMaxSize = 76800;        // 0x12C00 (整个flash大小)
                    appBaseAddr = 134248448;     // 0x08007800 (flash基址 + 30720)
                    argBaseAddr = 134247968;     // 0x08007620 (flash基址 + 30240)
                    readWriteSize = 240;         // 5300芯片使用240
                }
                else // 5800
                {
                    flashMaxSize = 262144;       // 0x40000 (256KB)
                    appBaseAddr = 134250496;     // 0x08008000 (flash基址 + 32768)
                    argBaseAddr = 134246400;     // 0x08007000 (flash基址 + 28672)
                    readWriteSize = 128;         // 5800芯片使用128
                }

                string jsonContent = $@"{{
  ""baudrate"": 115200,
  ""modbusAddr"": 2,
  ""appFile"": ""{appFile}"",
  ""flashBaseAddr"": {flashBaseAddr},
  ""flashMaxSize"": {flashMaxSize},
  ""appBaseAddr"": {appBaseAddr},
  ""appMaxSize"": {config.appMaxSize},
  ""argBaseAddr"": {argBaseAddr},
  ""readWriteSize"": {readWriteSize}
}}";

                File.WriteAllText(jsonPath, jsonContent);
                LogMessage($"创建{chipType}芯片IapTool {format}配置: {Path.GetFileName(jsonPath)}");
            }
            catch (Exception ex)
            {
                LogMessage($"创建IapTool JSON配置文件失败 {jsonPath}: {ex.Message}");
            }
        }

        // 新增方法：获取芯片配置
        private dynamic GetChipConfig(string chip)
        {
            if (chip == "5300")
            {
                return new
                {
                    bootMaxSize = 30720,
                    argBaseAddr = 30240,
                    appBaseAddr = 30720,
                    appMaxSize = 46080,
                    flashBaseAddr = 134217728,
                    bootFileName = "BootLoader-5300"
                };
            }
            else // 5800 默认
            {
                return new
                {
                    bootMaxSize = 32768,
                    argBaseAddr = 28672,
                    appBaseAddr = 32768,
                    appMaxSize = 229376,
                    flashBaseAddr = 134217728,
                    bootFileName = "BootLoader-5800"
                };
            }
        }

        // 新增方法：创建JSON配置文件
        private void CreateJsonConfig(string jsonPath, dynamic config, string appName, string format, bool includeOutFile)
        {
            try
            {
                string bootFile = $"{projectName}/{config.bootFileName}.{format.ToLower()}";
                string appFile = $"{projectName}/{appName}.{format.ToLower()}";

                string jsonContent;
                if (format == "HEX")
                {
                    if (includeOutFile)
                    {
                        string outFile = $"{projectName}/{appName.Replace("iap", "jlink")}.{format.ToLower()}";
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""outFile"": ""{outFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize},
  ""flashBaseAddr"": {config.flashBaseAddr}
}}";
                    }
                    else
                    {
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize},
  ""flashBaseAddr"": {config.flashBaseAddr}
}}";
                    }
                }
                else // BIN
                {
                    if (includeOutFile)
                    {
                        string outFile = $"{projectName}/{appName.Replace("iap", "jlink")}.{format.ToLower()}";
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""outFile"": ""{outFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize}
}}";
                    }
                    else
                    {
                        jsonContent = $@"{{
  ""bootFile"": ""{bootFile}"",
  ""appFile"": ""{appFile}"",
  ""bootMaxSize"": {config.bootMaxSize},
  ""argBaseAddr"": {config.argBaseAddr},
  ""appBaseAddr"": {config.appBaseAddr},
  ""appMaxSize"": {config.appMaxSize}
}}";
                    }
                }

                File.WriteAllText(jsonPath, jsonContent);
                LogMessage($"创建{chipType}芯片{format}配置: {Path.GetFileName(jsonPath)}");
            }
            catch (Exception ex)
            {
                LogMessage($"创建JSON配置文件失败 {jsonPath}: {ex.Message}");
            }
        }
        // ============================================================================
        // 总结：需要在MyData类中添加的内容
        // ============================================================================

        /*
        在MyData类中需要确保包含以下内容：

        1. 5300芯片的BootLoader数据：
           public static readonly byte[] BOOTLOADER_5300_BIN_DATA = { ... };
           public static readonly byte[] BOOTLOADER_5300_HEX_DATA = { ... };

        2. 可选：5300专用的批处理文件模板（如果需要特殊处理）：
           public const string ONLINE_BATCH_TEMPLATE_WITHOUT_STEP6_5300 = @"...";
           public const string ONLINE_BATCH_TEMPLATE_WITH_STEP6_5300 = @"...";

           或者，推荐的方法是修改现有模板，使其更通用化。

        3. 可选：5300专用的PowerShell脚本（如果需要）：
           public const string MERGE_IAP_FILES_PS1_CONTENT_5300 = @"...";

           但实际上PowerShell脚本是通用的，通过JSON配置传递参数，
           所以可能不需要专门的5300版本。

        建议的实现策略：
        - 优先使用C#程序直接创建JSON配置文件，而不是依赖批处理文件
        - 批处理文件主要负责编译和调用PowerShell脚本
        - PowerShell脚本通过JSON配置获取芯片特定的参数
        */

        // 16. 补充：确保BtnCreatePowerShellScript_Click方法支持芯片类型
        private void BtnCreatePowerShellScript_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                MessageBox.Show("请先选择工程文件夹！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            try
            {
                LogMessage($"=== 开始创建嵌入的PowerShell脚本 (芯片: {chipType}) ===");

                string psScriptPath = Path.Combine(projectPath, "MergeIapFiles.ps1");

                // PowerShell脚本是通用的，通过JSON配置获取参数，所以5300和5800使用相同的脚本
                string psScriptContent = MyData.MERGE_IAP_FILES_PS1_CONTENT;

                File.WriteAllText(psScriptPath, psScriptContent, new UTF8Encoding(false));
                LogMessage($"已创建PowerShell脚本: {psScriptPath}");

                LogMessage($"=== PowerShell脚本创建完成 (芯片: {chipType}) ===");
                lblStatus.Text = $"状态: PowerShell脚本已创建 ({chipType}芯片)";
                lblStatus.ForeColor = System.Drawing.Color.Green;

                MessageBox.Show($"PowerShell脚本创建成功！\n芯片类型: {chipType}\n现在可以进行编译合并操作。",
                    "成功", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                LogMessage($"创建PowerShell脚本失败: {ex.Message}");
                lblStatus.Text = "状态: PowerShell脚本创建失败";
                lblStatus.ForeColor = System.Drawing.Color.Red;
                MessageBox.Show($"创建PowerShell脚本失败: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // 17. 修改现有的BtnUserConfig_Click方法，集成新的JSON配置创建
        // （这个方法应该替换现有的BtnUserConfig_Click方法中的相关部分）
        private void IntegrateJsonConfigCreation()
        {
            // 在BtnUserConfig_Click方法的最后，在MessageBox.Show之前添加：

            if (rbOnlineMode.Checked)
            {
                // 预先创建JSON配置文件模板
                CreateDefaultJsonConfigs();
            }
        }

        private void BtnCreateBootLoaderFiles_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                MessageBox.Show("请先选择工程文件夹！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (string.IsNullOrEmpty(projectName))
            {
                MessageBox.Show("请输入项目名称！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            try
            {
                LogMessage($"=== 开始创建嵌入的BootLoader文件 (芯片类型: {chipType}) ===");

                var binData = GetCurrentBootLoaderBinData();
                var hexData = GetCurrentBootLoaderHexData();

                string iapDir = Path.Combine(projectPath, $@"IAP\IapFileCreator\{projectName}");
                Directory.CreateDirectory(iapDir);

                string binPath = Path.Combine(iapDir, $"BootLoader-{chipType}.bin");
                string hexPath = Path.Combine(iapDir, $"BootLoader-{chipType}.hex");

                File.WriteAllBytes(binPath, binData);
                LogMessage($"已创建: {binPath} ({binData.Length} bytes)");

                File.WriteAllBytes(hexPath, hexData);
                LogMessage($"已创建: {hexPath} ({hexData.Length} bytes)");

                LogMessage($"=== {chipType}芯片BootLoader文件创建完成 ===");
                lblStatus.Text = $"状态: {chipType}芯片BootLoader文件已创建";
                lblStatus.ForeColor = System.Drawing.Color.Green;

                MessageBox.Show($"{chipType}芯片BootLoader文件创建成功！\n现在可以进行编译合并操作。", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                LogMessage($"创建{chipType}芯片BootLoader文件失败: {ex.Message}");
                lblStatus.Text = $"状态: {chipType}芯片BootLoader文件创建失败";
                lblStatus.ForeColor = System.Drawing.Color.Red;
                MessageBox.Show($"创建{chipType}芯片BootLoader文件失败: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }



        private void BtnSelectProject_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog dialog = new FolderBrowserDialog())
            {
                dialog.Description = "选择项目根目录 (程序会自动寻找FLASH目录)";
                dialog.ShowNewFolderButton = false;

                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    string selectedPath = dialog.SelectedPath;
                    LogMessage($"用户选择的路径: {selectedPath}");

                    // 尝试自动识别项目结构
                    string actualProjectPath = FindProjectFlashDirectory(selectedPath);

                    if (!string.IsNullOrEmpty(actualProjectPath))
                    {
                        projectPath = actualProjectPath;
                        txtProjectPath.Text = $"{selectedPath} → {Path.GetFileName(actualProjectPath)}";
                        LogMessage($"✅ 找到项目FLASH目录: {actualProjectPath}");

                        ValidateProjectPath();
                        LoadCurrentConfiguration();
                    }
                    else
                    {
                        LogMessage($"❌ 在选择的目录中未找到有效的项目结构");
                        MessageBox.Show(
                            $"在选择的目录中未找到有效的项目结构！\n\n" +
                            $"请确保选择的是包含以下结构的项目根目录：\n" +
                            $"• ProjectTemplate\\Project\\MDK\\FLASH\\\n" +
                            $"• Project\\MDK\\FLASH\\\n" +
                            $"• MDK\\FLASH\\\n" +
                            $"• FLASH\\",
                            "未找到项目结构",
                            MessageBoxButtons.OK,
                            MessageBoxIcon.Warning);

                        txtProjectPath.Text = "请选择正确的项目根目录...";
                        projectPath = "";
                    }
                }
            }
        }

        private void TxtProjectName_TextChanged(object sender, EventArgs e)
        {
            projectName = txtProjectName.Text.Trim();
            if (!string.IsNullOrEmpty(projectName))
            {
                LogMessage($"项目名称已更新为: {projectName}");
            }
        }

        private void ValidateProjectPath()
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                lblStatus.Text = "状态: 未选择工程";
                lblStatus.ForeColor = System.Drawing.Color.Red;
                return;
            }

            string scatterFileName = GetScatterFileName(chipType);
            string scatterPath = Path.Combine(projectPath, scatterFileName);

            if (File.Exists(scatterPath))
            {
                lblStatus.Text = "状态: 工程路径有效";
                lblStatus.ForeColor = System.Drawing.Color.Green;
                LogMessage($"✓ 找到{chipType}芯片scatter文件: {scatterFileName}");
            }
            else
            {
                lblStatus.Text = $"状态: 工程路径无效 (找不到{chipType}芯片的.sct文件)";
                lblStatus.ForeColor = System.Drawing.Color.Orange;
                LogMessage($"⚠ 未找到{chipType}芯片scatter文件: {scatterPath}");

                // 尝试查找其他芯片类型的scatter文件
                string otherChipType = chipType == "5300" ? "5800" : "5300";
                string otherScatterFileName = GetScatterFileName(otherChipType);
                string otherScatterPath = Path.Combine(projectPath, otherScatterFileName);

                if (File.Exists(otherScatterPath))
                {
                    LogMessage($"💡 但找到了{otherChipType}芯片的scatter文件: {otherScatterFileName}");
                    LogMessage($"建议切换芯片类型为{otherChipType}，或检查项目是否为{chipType}芯片类型");
                }
            }

            string[] requiredDirs = { "Objects", "Execute" };
            foreach (string dir in requiredDirs)
            {
                string dirPath = Path.Combine(projectPath, dir);
                if (Directory.Exists(dirPath))
                {
                    LogMessage($"✓ 找到目录: {dir}");
                }
                else
                {
                    LogMessage($"⚠ 未找到目录: {dir}");
                }
            }
        }


        private void BtnQuickFix_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                MessageBox.Show("请先选择工程文件夹！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            try
            {
                LogMessage($"=== 开始快速修复{chipType}芯片scatter文件格式 ===");

                string scatterFileName = GetScatterFileName(chipType);
                string scatterPath = Path.Combine(projectPath, scatterFileName);

                if (!File.Exists(scatterPath))
                {
                    LogMessage($"错误: {chipType}芯片Scatter文件不存在: {scatterPath}");
                    MessageBox.Show($"{chipType}芯片Scatter文件不存在！", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                string backupPath = scatterPath + ".quickfix_backup";
                File.Copy(scatterPath, backupPath, true);
                LogMessage($"已备份原文件到: {backupPath}");

                string content = File.ReadAllText(scatterPath, new UTF8Encoding(false));

                LogMessage("转换分号注释为块注释...");

                content = Regex.Replace(content,
                    @"^(\s*);(#define\s+__ROM_BASE\s+0x[0-9A-Fa-f]+)(.*)$",
                    "$1/*$2*/$3",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);(#define\s+__ROM_SIZE\s+0x[0-9A-Fa-f]+)(.*)$",
                    "$1/*$2*/$3",
                    RegexOptions.Multiline);

                LogMessage("修复完成，转换的配置:");
                LogMessage("  - ;#define __ROM_BASE ... → /*#define __ROM_BASE ...*/");
                LogMessage("  - ;#define __ROM_SIZE ... → /*#define __ROM_SIZE ...*/");

                File.WriteAllText(scatterPath, content, new UTF8Encoding(false));

                LogMessage($"=== {chipType}芯片快速修复完成 ===");
                lblStatus.Text = $"状态: {chipType}芯片快速修复完成";
                lblStatus.ForeColor = System.Drawing.Color.Green;

                LoadCurrentConfiguration();

                MessageBox.Show($"{chipType}芯片快速修复完成！\n已将分号注释转换为块注释格式。\n请重新编译项目测试。",
                    "修复完成", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                LogMessage($"快速修复失败: {ex.Message}");
                lblStatus.Text = "状态: 快速修复失败";
                lblStatus.ForeColor = System.Drawing.Color.Red;
                MessageBox.Show($"快速修复失败: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void RbMode_CheckedChanged(object sender, EventArgs e)
        {
            RadioButton rb = sender as RadioButton;
            if (rb.Checked)
            {
                LogMessage($"选择模式: {rb.Text}");

                if (rb == rbOnlineMode)
                {
                    LogMessage("注意: 在线模式支持编译合并功能");
                    chkReplaceIapFiles.Enabled = true;
                }
                else if (rb == rbOfflineMode)
                {
                    LogMessage("注意: 非在线模式不需要编译合并");
                    btnCompile.Enabled = false;
                    chkReplaceIapFiles.Enabled = false;
                    chkReplaceIapFiles.Checked = false;
                }
            }
        }

        private void ChkReplaceIapFiles_CheckedChanged(object sender, EventArgs e)
        {
            CheckBox cb = sender as CheckBox;
            if (cb.Checked)
            {
                LogMessage("已启用: 替换iap文件功能 (将执行Step 6: 拷贝jlink文件到Execute目录)");
            }
            else
            {
                LogMessage("已禁用: 替换iap文件功能 (使用原始批处理流程)");
            }
        }


        private void CreateIAPDirectories()
        {
            try
            {
                string iapPath1 = Path.Combine(projectPath, $@"IAP\IapFileCreator\{projectName}");
                string iapPath2 = Path.Combine(projectPath, $@"IAP\IapTool\{projectName}");

                Directory.CreateDirectory(iapPath1);
                Directory.CreateDirectory(iapPath2);

                LogMessage($"创建目录: {iapPath1}");
                LogMessage($"创建目录: {iapPath2}");
            }
            catch (Exception ex)
            {
                LogMessage($"创建IAP目录失败: {ex.Message}");
            }
        }

        private bool ValidateScatterFileFormat(string content)
        {
            string[] lines = content.Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);

            if (lines.Length == 0)
            {
                LogMessage("错误: Scatter文件为空");
                return false;
            }

            string firstLine = lines[0].Trim();
            if (!firstLine.StartsWith("#!") || !firstLine.Contains("armclang"))
            {
                LogMessage($"错误: 第一行格式不正确: {firstLine}");
                LogMessage("第一行应该是: #! armclang -E --target=arm-arm-none-eabi -mcpu=cortex-m3 -xc");
                return false;
            }

            LogMessage("Scatter文件格式验证通过");
            return true;
        }

        private string ConfigureOnlineMode(string content)
        {
            LogMessage("正在转换为在线升级模式配置...");

            if (chipType == "5800")
            {
                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_BASE\s+0x08000000.*?$",
                    "$1/*#define __ROM_BASE      0x08000000*/",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_SIZE\s+0x00020000.*?$",
                    "$1/*#define __ROM_SIZE      0x00020000*/",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_BASE\s+0x08008000(?:\*/)?.*?$",
                    "$1#define __ROM_BASE      0x08008000",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_SIZE\s+0x38000(?:\*/)?.*?$",
                    "$1#define __ROM_SIZE      0x38000",
                    RegexOptions.Multiline);

                LogMessage("  - 屏蔽: ROM_BASE 0x08000000");
                LogMessage("  - 激活: ROM_BASE 0x08008000 (IAP App区域)");
                LogMessage("  - 屏蔽: ROM_SIZE 0x00020000");
                LogMessage("  - 激活: ROM_SIZE 0x38000 (229KB)");
            }
            else if (chipType == "5300")
            {
                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_BASE\s+0x08000000.*?$",
                    "$1/*#define __ROM_BASE      0x08000000*/",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_SIZE\s+0x000129FF.*?$",
                    "$1/*#define __ROM_SIZE      0x000129FF*/",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_BASE\s+0x08007800(?:\*/)?.*?$",
                    "$1#define __ROM_BASE      0x08007800",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_SIZE\s+0xB400(?:\*/)?.*?$",
                    "$1#define __ROM_SIZE      0xB400",
                    RegexOptions.Multiline);

                LogMessage("  - 屏蔽: ROM_BASE 0x08000000");
                LogMessage("  - 激活: ROM_BASE 0x08007800 (IAP App区域)");
                LogMessage("  - 屏蔽: ROM_SIZE 0x000129FF");
                LogMessage("  - 激活: ROM_SIZE 0xB400 (45.5KB)");
            }

            return content;
        }


        private string ConfigureOfflineMode(string content)
        {
            LogMessage("正在转换为非在线升级模式配置...");

            if (chipType == "5800")
            {
                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_BASE\s+0x08000000(?:\*/)?.*?$",
                    "$1#define __ROM_BASE      0x08000000",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_SIZE\s+0x00020000(?:\*/)?.*?$",
                    "$1#define __ROM_SIZE      0x00020000",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_BASE\s+0x08008000.*?$",
                    "$1/*#define __ROM_BASE      0x08008000*/",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_SIZE\s+0x38000.*?$",
                    "$1/*#define __ROM_SIZE      0x38000*/",
                    RegexOptions.Multiline);

                LogMessage("  - 激活: ROM_BASE 0x08000000 (完整Flash)");
                LogMessage("  - 屏蔽: ROM_BASE 0x08008000");
                LogMessage("  - 激活: ROM_SIZE 0x00020000 (128KB)");
                LogMessage("  - 屏蔽: ROM_SIZE 0x38000");
            }
            else if (chipType == "5300")
            {
                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_BASE\s+0x08000000(?:\*/)?.*?$",
                    "$1#define __ROM_BASE      0x08000000",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*)(?:;|/\*)?#define __ROM_SIZE\s+0x000129FF(?:\*/)?.*?$",
                    "$1#define __ROM_SIZE      0x000129FF",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_BASE\s+0x08007800.*?$",
                    "$1/*#define __ROM_BASE      0x08007800*/",
                    RegexOptions.Multiline);

                content = Regex.Replace(content,
                    @"^(\s*);?#define __ROM_SIZE\s+0xB400.*?$",
                    "$1/*#define __ROM_SIZE      0xB400*/",
                    RegexOptions.Multiline);

                LogMessage("  - 激活: ROM_BASE 0x08000000 (完整Flash)");
                LogMessage("  - 屏蔽: ROM_BASE 0x08007800");
                LogMessage("  - 激活: ROM_SIZE 0x000129FF (76KB)");
                LogMessage("  - 屏蔽: ROM_SIZE 0xB400");
            }

            return content;
        }

        private void BtnCompile_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                MessageBox.Show("请先选择工程文件夹！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (!rbOnlineMode.Checked)
            {
                MessageBox.Show("只有在线升级模式需要编译合并！", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            try
            {
                LogMessage("=== 开始编译和合并 ===");
                LogMessage($"当前芯片类型: {chipType}");

                string batchPath = Path.Combine(projectPath, BATCH_FILE);

                if (!File.Exists(batchPath))
                {
                    LogMessage($"错误: 批处理文件不存在: {batchPath}");
                    MessageBox.Show($"找不到批处理文件: {BATCH_FILE}\n请确保文件在项目目录中", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                if (chkReplaceIapFiles.Checked)
                {
                    LogMessage("使用增强版批处理 (包含Step 6: 拷贝jlink文件)");
                }
                else
                {
                    LogMessage("使用标准版批处理 (不包含Step 6)");
                }

                LogMessage($"批处理文件将自动确保{chipType}芯片BootLoader文件存在");

                ProcessStartInfo psi = new ProcessStartInfo
                {
                    FileName = "cmd.exe",
                    Arguments = $"/c \"{batchPath}\"",
                    WorkingDirectory = projectPath,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                };

                LogMessage($"执行命令: {psi.Arguments}");
                LogMessage($"工作目录: {psi.WorkingDirectory}");

                using (Process process = Process.Start(psi))
                {
                    process.OutputDataReceived += (s, e) =>
                    {
                        if (!string.IsNullOrEmpty(e.Data))
                        {
                            this.Invoke(new Action(() => LogMessage($"[输出] {e.Data}")));
                        }
                    };

                    process.ErrorDataReceived += (s, e) =>
                    {
                        if (!string.IsNullOrEmpty(e.Data))
                        {
                            this.Invoke(new Action(() => LogMessage($"[错误] {e.Data}")));
                        }
                    };

                    process.BeginOutputReadLine();
                    process.BeginErrorReadLine();

                    process.WaitForExit();

                    if (process.ExitCode == 0)
                    {
                        LogMessage("=== 编译合并完成 ===");
                        lblStatus.Text = $"状态: 编译合并成功 ({chipType}芯片)";
                        lblStatus.ForeColor = System.Drawing.Color.Green;
                        MessageBox.Show($"编译和合并操作完成！\n芯片类型: {chipType}", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    }
                    else
                    {
                        LogMessage($"=== 编译合并失败，退出代码: {process.ExitCode} ===");
                        lblStatus.Text = "状态: 编译合并失败";
                        lblStatus.ForeColor = System.Drawing.Color.Red;
                        MessageBox.Show($"编译合并失败，退出代码: {process.ExitCode}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage($"编译合并出错: {ex.Message}");
                lblStatus.Text = "状态: 编译合并出错";
                lblStatus.ForeColor = System.Drawing.Color.Red;
                MessageBox.Show($"编译合并出错: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        // 创建基本目录结构
        private void CreateBasicDirectories()
        {
            try
            {
                // 创建Execute目录
                string executeDir = Path.Combine(projectPath, "Execute");
                if (!Directory.Exists(executeDir))
                {
                    Directory.CreateDirectory(executeDir);
                    LogMessage($"✅ 已创建目录: Execute");
                }
                else
                {
                    LogMessage($"✅ Execute目录已存在");
                }

                // 创建Objects目录（如果不存在）
                string objectsDir = Path.Combine(projectPath, "Objects");
                if (!Directory.Exists(objectsDir))
                {
                    Directory.CreateDirectory(objectsDir);
                    LogMessage($"✅ 已创建目录: Objects");
                }
                else
                {
                    LogMessage($"✅ Objects目录已存在");
                }

                // 确保IAP根目录存在
                string iapDir = Path.Combine(projectPath, "IAP");
                if (!Directory.Exists(iapDir))
                {
                    Directory.CreateDirectory(iapDir);
                    LogMessage($"✅ 已创建目录: IAP");
                }
                else
                {
                    LogMessage($"✅ IAP目录已存在");
                }
            }
            catch (Exception ex)
            {
                LogMessage($"创建基本目录失败: {ex.Message}");
                throw;
            }
        }

        // 自动查找项目的FLASH目录
        private string FindProjectFlashDirectory(string rootPath)
        {
            try
            {
                // 定义可能的路径模式（按优先级排序）
                string[] possiblePaths = {
            @"ProjectTemplate\Project\MDK\FLASH",  // 标准完整路径
            @"Project\MDK\FLASH",                  // 简化路径1
            @"MDK\FLASH",                          // 简化路径2
            @"FLASH"                               // 最简路径
        };

                foreach (string relativePath in possiblePaths)
                {
                    string fullPath = Path.Combine(rootPath, relativePath);
                    LogMessage($"检查路径: {fullPath}");

                    if (Directory.Exists(fullPath))
                    {
                        // 首先检查当前选择的芯片类型的scatter文件
                        string currentScatterFile = Path.Combine(fullPath, GetScatterFileName(chipType));
                        if (File.Exists(currentScatterFile))
                        {
                            LogMessage($"✅ 找到{chipType}芯片scatter文件: {currentScatterFile}");
                            return fullPath;
                        }

                        // 如果当前芯片类型的scatter文件不存在，检查其他芯片类型
                        string otherChipType = chipType == "5300" ? "5800" : "5300";
                        string otherScatterFile = Path.Combine(fullPath, GetScatterFileName(otherChipType));
                        if (File.Exists(otherScatterFile))
                        {
                            LogMessage($"⚠️ 目录存在但只找到{otherChipType}芯片的scatter文件: {otherScatterFile}");
                            LogMessage($"💡 建议切换芯片类型为{otherChipType}");
                            // 仍然返回路径，但会在ValidateProjectPath中显示警告
                            return fullPath;
                        }
                        else
                        {
                            LogMessage($"⚠️ 目录存在但缺少scatter文件");
                        }
                    }
                }

                // 如果以上都不行，尝试递归搜索（限制深度）
                LogMessage("尝试递归搜索FLASH目录...");
                return SearchFlashDirectoryRecursively(rootPath, 0, 4); // 最大深度4层
            }
            catch (Exception ex)
            {
                LogMessage($"查找项目目录时出错: {ex.Message}");
                return null;
            }
        }

        // 递归搜索FLASH目录
        private string SearchFlashDirectoryRecursively(string currentPath, int currentDepth, int maxDepth)
        {
            if (currentDepth > maxDepth)
                return null;

            try
            {
                // 检查当前目录是否是FLASH目录
                if (Path.GetFileName(currentPath).Equals("FLASH", StringComparison.OrdinalIgnoreCase))
                {
                    // 首先检查当前选择的芯片类型
                    string currentScatterFile = Path.Combine(currentPath, GetScatterFileName(chipType));
                    if (File.Exists(currentScatterFile))
                    {
                        LogMessage($"🎯 递归找到{chipType}芯片FLASH目录: {currentPath}");
                        return currentPath;
                    }

                    // 检查其他芯片类型
                    string otherChipType = chipType == "5300" ? "5800" : "5300";
                    string otherScatterFile = Path.Combine(currentPath, GetScatterFileName(otherChipType));
                    if (File.Exists(otherScatterFile))
                    {
                        LogMessage($"🎯 递归找到FLASH目录，但只有{otherChipType}芯片文件: {currentPath}");
                        return currentPath;
                    }
                }

                // 搜索子目录
                string[] subDirectories = Directory.GetDirectories(currentPath);
                foreach (string subDir in subDirectories)
                {
                    string result = SearchFlashDirectoryRecursively(subDir, currentDepth + 1, maxDepth);
                    if (!string.IsNullOrEmpty(result))
                        return result;
                }
            }
            catch (Exception ex)
            {
                LogMessage($"递归搜索出错 ({currentPath}): {ex.Message}");
            }

            return null;
        }
        private void LogMessage(string message)
        {
            if (txtLog.InvokeRequired)
            {
                txtLog.Invoke(new Action<string>(LogMessage), message);
                return;
            }

            string timestamp = DateTime.Now.ToString("HH:mm:ss");
            txtLog.AppendText($"[{timestamp}] {message}\r\n");
            txtLog.ScrollToCaret();
        }
        private string GetScatterFileName(string chipType)
        {
            switch (chipType)
            {
                case "5300":
                    return "tae32f53xx_ac6_flash.sct";  // 5300芯片的scatter文件名
                case "5800":
                default:
                    return "tae32g58xx_ac6_flash.sct";  // 5800芯片的scatter文件名
            }
        }
    }

    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}