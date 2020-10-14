using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Media;
using TrafficOptimizerUI.logic;
using TrafficOptimizerUI.UI;

namespace TrafficOptimizerUI
{
    public partial class FormHome : Form, OptimizationFinishListener
    {
        private delegate void SafeButtonEnable(Button button);



        public FormHome()
        {
            InitializeComponent();
        }



        private void buttonStartOptimization_Click(object sender, EventArgs e)
        {
            buttonStartOptimization.Enabled = false;
            FormStartOptimization form = new FormStartOptimization(this);
            DialogResult res = form.ShowDialog();
            if (res == DialogResult.Cancel)
                buttonStartOptimization.Enabled = true;
            else if (res == DialogResult.OK)
                this.WindowState = FormWindowState.Minimized;
        }



        private void buttonDisplayResult_Click(object sender, EventArgs e)
        {
            openFileDialogResultFile.ShowDialog();
            String path = openFileDialogResultFile.FileName;
            if (!String.IsNullOrEmpty(path))
            {
                OptimizerProxy optimizer = new OptimizerProxy();
                OptimizationResult res = optimizer.ReadResult(path);
                if (res != null)
                {
                    String[] pathSplitted = path.Split(Path.DirectorySeparatorChar);
                    Form resultForm = new FormShowResult(res, 
                        pathSplitted[pathSplitted.Length - 1]);
                    resultForm.ShowDialog();
                }
                else
                {
                    MessageBox.Show(Properties.Resources.CantReadResult);
                }

                openFileDialogResultFile.FileName = null;
            }
        }



        public void Finished()
        {
            enableButton(buttonStartOptimization);
        }



        private void enableButton(Button button)
        {
            if (buttonStartOptimization.InvokeRequired)
            {
                var d = new SafeButtonEnable(enableButton);
                button.Invoke(d, new object[] { button });
            }
            else
            {
                button.Enabled = true;
            }
        }
    }
}
