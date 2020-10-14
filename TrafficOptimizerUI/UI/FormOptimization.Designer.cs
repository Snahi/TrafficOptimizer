namespace TrafficOptimizerUI
{
    partial class FormOptimization
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormOptimization));
            this.cartesianChart1 = new LiveCharts.WinForms.CartesianChart();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.labelOptimizing = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.labelRemainingTime = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.labelStopping = new System.Windows.Forms.Label();
            this.progressBarStop = new System.Windows.Forms.ProgressBar();
            this.buttonSave = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label8 = new System.Windows.Forms.Label();
            this.comboBoxParam1 = new System.Windows.Forms.ComboBox();
            this.labelBestSolution = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.saveFileDialogSaveCurrent = new System.Windows.Forms.SaveFileDialog();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // cartesianChart1
            // 
            this.cartesianChart1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.cartesianChart1.Location = new System.Drawing.Point(23, 66);
            this.cartesianChart1.Name = "cartesianChart1";
            this.cartesianChart1.Size = new System.Drawing.Size(1763, 645);
            this.cartesianChart1.TabIndex = 0;
            this.cartesianChart1.Text = "cartesianChart1";
            // 
            // progressBar
            // 
            this.progressBar.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBar.Location = new System.Drawing.Point(22, 52);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(1760, 23);
            this.progressBar.TabIndex = 1;
            // 
            // labelOptimizing
            // 
            this.labelOptimizing.AutoSize = true;
            this.labelOptimizing.Location = new System.Drawing.Point(19, 32);
            this.labelOptimizing.Name = "labelOptimizing";
            this.labelOptimizing.Size = new System.Drawing.Size(126, 17);
            this.labelOptimizing.TabIndex = 2;
            this.labelOptimizing.Text = "Optymalizowanie...";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(19, 78);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(110, 17);
            this.label2.TabIndex = 3;
            this.label2.Text = "Pozostały czas: ";
            // 
            // labelRemainingTime
            // 
            this.labelRemainingTime.AutoSize = true;
            this.labelRemainingTime.Location = new System.Drawing.Point(129, 78);
            this.labelRemainingTime.Name = "labelRemainingTime";
            this.labelRemainingTime.Size = new System.Drawing.Size(86, 17);
            this.labelRemainingTime.TabIndex = 4;
            this.labelRemainingTime.Text = "Obliczanie...";
            this.toolTip1.SetToolTip(this.labelRemainingTime, "Szacunkowy czas pozostały do zakończenia optymalizacji jeżeli optimum nie zostani" +
        "e znalezione wcześniej");
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.labelStopping);
            this.groupBox1.Controls.Add(this.progressBarStop);
            this.groupBox1.Controls.Add(this.buttonSave);
            this.groupBox1.Controls.Add(this.buttonStop);
            this.groupBox1.Controls.Add(this.labelOptimizing);
            this.groupBox1.Controls.Add(this.progressBar);
            this.groupBox1.Controls.Add(this.labelRemainingTime);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(1803, 153);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Postęp";
            // 
            // labelStopping
            // 
            this.labelStopping.AutoSize = true;
            this.labelStopping.Location = new System.Drawing.Point(143, 116);
            this.labelStopping.Name = "labelStopping";
            this.labelStopping.Size = new System.Drawing.Size(114, 17);
            this.labelStopping.TabIndex = 11;
            this.labelStopping.Text = "Zatrzymywanie...";
            this.labelStopping.Visible = false;
            // 
            // progressBarStop
            // 
            this.progressBarStop.Location = new System.Drawing.Point(69, 115);
            this.progressBarStop.Name = "progressBarStop";
            this.progressBarStop.Size = new System.Drawing.Size(68, 19);
            this.progressBarStop.TabIndex = 10;
            this.progressBarStop.Visible = false;
            // 
            // buttonSave
            // 
            this.buttonSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSave.Location = new System.Drawing.Point(1707, 104);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 40);
            this.buttonSave.TabIndex = 8;
            this.buttonSave.Text = "Zapisz";
            this.toolTip1.SetToolTip(this.buttonSave, "Zapisz aktualny wynik bez przerywania optymalizacji");
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("buttonStop.BackgroundImage")));
            this.buttonStop.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.buttonStop.Location = new System.Drawing.Point(22, 104);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(40, 40);
            this.buttonStop.TabIndex = 7;
            this.toolTip1.SetToolTip(this.buttonStop, "Zapisz aktualny wynik i zakończ optymalizację");
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.comboBoxParam1);
            this.groupBox2.Controls.Add(this.labelBestSolution);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.cartesianChart1);
            this.groupBox2.Location = new System.Drawing.Point(9, 171);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(1809, 717);
            this.groupBox2.TabIndex = 8;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Statystyki";
            // 
            // label8
            // 
            this.label8.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(1287, 37);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(144, 17);
            this.label8.TabIndex = 14;
            this.label8.Text = "Parametr na wykresie";
            // 
            // comboBoxParam1
            // 
            this.comboBoxParam1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxParam1.FormattingEnabled = true;
            this.comboBoxParam1.Location = new System.Drawing.Point(1443, 33);
            this.comboBoxParam1.Name = "comboBoxParam1";
            this.comboBoxParam1.Size = new System.Drawing.Size(360, 24);
            this.comboBoxParam1.TabIndex = 12;
            this.toolTip1.SetToolTip(this.comboBoxParam1, "Wybierz parameter który ma zostać wyświetlony na wykresie");
            this.comboBoxParam1.SelectedIndexChanged += new System.EventHandler(this.comboBoxParam1_SelectedIndexChanged);
            // 
            // labelBestSolution
            // 
            this.labelBestSolution.AutoSize = true;
            this.labelBestSolution.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelBestSolution.Location = new System.Drawing.Point(238, 36);
            this.labelBestSolution.Name = "labelBestSolution";
            this.labelBestSolution.Size = new System.Drawing.Size(23, 18);
            this.labelBestSolution.TabIndex = 11;
            this.labelBestSolution.Text = "...";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(19, 36);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(203, 18);
            this.label6.TabIndex = 10;
            this.label6.Text = "Aktualny najlepszy wynik: ";
            // 
            // saveFileDialogSaveCurrent
            // 
            this.saveFileDialogSaveCurrent.DefaultExt = "xml";
            // 
            // FormOptimization
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1827, 895);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Name = "FormOptimization";
            this.Text = "FormOptimization";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private LiveCharts.WinForms.CartesianChart cartesianChart1;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Label labelOptimizing;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label labelRemainingTime;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label labelBestSolution;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox comboBoxParam1;
        private System.Windows.Forms.SaveFileDialog saveFileDialogSaveCurrent;
        private System.Windows.Forms.ProgressBar progressBarStop;
        private System.Windows.Forms.Label labelStopping;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}