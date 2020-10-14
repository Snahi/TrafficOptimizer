namespace TrafficOptimizerUI
{
    partial class FormHome
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
            this.buttonStartOptimization = new System.Windows.Forms.Button();
            this.buttonDisplayResult = new System.Windows.Forms.Button();
            this.openFileDialogResultFile = new System.Windows.Forms.OpenFileDialog();
            this.SuspendLayout();
            // 
            // buttonStartOptimization
            // 
            this.buttonStartOptimization.Location = new System.Drawing.Point(26, 173);
            this.buttonStartOptimization.Name = "buttonStartOptimization";
            this.buttonStartOptimization.Size = new System.Drawing.Size(435, 49);
            this.buttonStartOptimization.TabIndex = 0;
            this.buttonStartOptimization.Text = "Rozpocznij optymalizację";
            this.buttonStartOptimization.UseVisualStyleBackColor = true;
            this.buttonStartOptimization.Click += new System.EventHandler(this.buttonStartOptimization_Click);
            // 
            // buttonDisplayResult
            // 
            this.buttonDisplayResult.Location = new System.Drawing.Point(26, 228);
            this.buttonDisplayResult.Name = "buttonDisplayResult";
            this.buttonDisplayResult.Size = new System.Drawing.Size(435, 49);
            this.buttonDisplayResult.TabIndex = 2;
            this.buttonDisplayResult.Text = "Wyświetl wynik optymalizacji";
            this.buttonDisplayResult.UseVisualStyleBackColor = true;
            this.buttonDisplayResult.Click += new System.EventHandler(this.buttonDisplayResult_Click);
            // 
            // FormHome
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(487, 450);
            this.Controls.Add(this.buttonDisplayResult);
            this.Controls.Add(this.buttonStartOptimization);
            this.Name = "FormHome";
            this.Text = "Ekran główny";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button buttonStartOptimization;
        private System.Windows.Forms.Button buttonDisplayResult;
        private System.Windows.Forms.OpenFileDialog openFileDialogResultFile;
    }
}