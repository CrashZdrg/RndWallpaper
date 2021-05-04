using Microsoft.Win32;
using System;
using System.IO;
using System.Windows.Forms;

namespace RndWallpaper.Config
{
    public partial class MainForm : Form
    {
        const string FOLDERPATH_FILE = "FolderPath.txt";
        const string REGISTRYKEY = nameof(RndWallpaper);
        const string APPFILE = "RndWallpaper.exe";

        RegistryKey runRK;
        bool isEnabled;
        string appPath;

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            try
            {
                if (File.Exists(FOLDERPATH_FILE))
                    txbPath.Text = File.ReadAllText(FOLDERPATH_FILE);

                runRK = Registry.CurrentUser.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true);

                isEnabled = runRK.GetValue(REGISTRYKEY) != null;

                appPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, APPFILE);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                Enabled = false;
            }

            UpdateEnDisButton();
        }

        private void BtnEnDis_Click(object sender, EventArgs e)
        {
            if (isEnabled)
                runRK.DeleteValue(REGISTRYKEY, false);
            else
                runRK.SetValue(REGISTRYKEY, appPath);

            isEnabled = !isEnabled;
            UpdateEnDisButton();
        }

        private void BtnPath_Click(object sender, EventArgs e)
        {
            using (var dialog = new FolderBrowserDialog())
                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    txbPath.Text = dialog.SelectedPath;
                    SaveFolderPath();
                }
        }

        private void TxbPath_Leave(object sender, EventArgs e) => SaveFolderPath();

        private void MainForm_FormClosed(object sender, FormClosedEventArgs e) => runRK?.Dispose();

        private void SaveFolderPath()
        {
            try
            {
                File.WriteAllText(FOLDERPATH_FILE, txbPath.Text);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void UpdateEnDisButton() => btnEnDis.Text = isEnabled ? "Disable" : "Enable";
    }
}
