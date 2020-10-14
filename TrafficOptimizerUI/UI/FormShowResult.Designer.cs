namespace TrafficOptimizerUI.UI
{
    partial class FormShowResult
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
            this.groupBoxDurations = new System.Windows.Forms.GroupBox();
            this.dataGridViewDurations = new System.Windows.Forms.DataGridView();
            this.lights_group_id = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Duration = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.groupBoxOffsets = new System.Windows.Forms.GroupBox();
            this.dataGridViewOffsets = new System.Windows.Forms.DataGridView();
            this.lights_system_id = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.offset = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.label1 = new System.Windows.Forms.Label();
            this.labelResult = new System.Windows.Forms.Label();
            this.groupBoxDurations.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewDurations)).BeginInit();
            this.groupBoxOffsets.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewOffsets)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBoxDurations
            // 
            this.groupBoxDurations.Controls.Add(this.dataGridViewDurations);
            this.groupBoxDurations.Location = new System.Drawing.Point(17, 51);
            this.groupBoxDurations.Name = "groupBoxDurations";
            this.groupBoxDurations.Size = new System.Drawing.Size(351, 436);
            this.groupBoxDurations.TabIndex = 0;
            this.groupBoxDurations.TabStop = false;
            this.groupBoxDurations.Text = "Czasy trwania";
            // 
            // dataGridViewDurations
            // 
            this.dataGridViewDurations.AllowUserToAddRows = false;
            this.dataGridViewDurations.AllowUserToDeleteRows = false;
            this.dataGridViewDurations.AllowUserToOrderColumns = true;
            this.dataGridViewDurations.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewDurations.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.lights_group_id,
            this.Duration});
            this.dataGridViewDurations.Location = new System.Drawing.Point(6, 21);
            this.dataGridViewDurations.Name = "dataGridViewDurations";
            this.dataGridViewDurations.ReadOnly = true;
            this.dataGridViewDurations.RowHeadersWidth = 51;
            this.dataGridViewDurations.RowTemplate.Height = 24;
            this.dataGridViewDurations.Size = new System.Drawing.Size(336, 410);
            this.dataGridViewDurations.TabIndex = 0;
            // 
            // lights_group_id
            // 
            this.lights_group_id.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.lights_group_id.HeaderText = "Grupa";
            this.lights_group_id.MinimumWidth = 6;
            this.lights_group_id.Name = "lights_group_id";
            this.lights_group_id.ReadOnly = true;
            // 
            // Duration
            // 
            this.Duration.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.Duration.HeaderText = "Czas trwania [s]";
            this.Duration.MinimumWidth = 6;
            this.Duration.Name = "Duration";
            this.Duration.ReadOnly = true;
            // 
            // groupBoxOffsets
            // 
            this.groupBoxOffsets.Controls.Add(this.dataGridViewOffsets);
            this.groupBoxOffsets.Location = new System.Drawing.Point(17, 493);
            this.groupBoxOffsets.Name = "groupBoxOffsets";
            this.groupBoxOffsets.Size = new System.Drawing.Size(351, 262);
            this.groupBoxOffsets.TabIndex = 1;
            this.groupBoxOffsets.TabStop = false;
            this.groupBoxOffsets.Text = "Fazy";
            // 
            // dataGridViewOffsets
            // 
            this.dataGridViewOffsets.AllowUserToAddRows = false;
            this.dataGridViewOffsets.AllowUserToDeleteRows = false;
            this.dataGridViewOffsets.AllowUserToOrderColumns = true;
            this.dataGridViewOffsets.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewOffsets.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.lights_system_id,
            this.offset});
            this.dataGridViewOffsets.Location = new System.Drawing.Point(6, 21);
            this.dataGridViewOffsets.Name = "dataGridViewOffsets";
            this.dataGridViewOffsets.ReadOnly = true;
            this.dataGridViewOffsets.RowHeadersWidth = 51;
            this.dataGridViewOffsets.RowTemplate.Height = 24;
            this.dataGridViewOffsets.Size = new System.Drawing.Size(336, 235);
            this.dataGridViewOffsets.TabIndex = 0;
            // 
            // lights_system_id
            // 
            this.lights_system_id.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.lights_system_id.HeaderText = "System świateł";
            this.lights_system_id.MinimumWidth = 6;
            this.lights_system_id.Name = "lights_system_id";
            this.lights_system_id.ReadOnly = true;
            // 
            // offset
            // 
            this.offset.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.offset.HeaderText = "Faza [s]";
            this.offset.MinimumWidth = 6;
            this.offset.Name = "offset";
            this.offset.ReadOnly = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.2F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(20, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(59, 20);
            this.label1.TabIndex = 2;
            this.label1.Text = "Wynik:";
            // 
            // labelResult
            // 
            this.labelResult.AutoSize = true;
            this.labelResult.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.2F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelResult.Location = new System.Drawing.Point(95, 9);
            this.labelResult.Name = "labelResult";
            this.labelResult.Size = new System.Drawing.Size(24, 20);
            this.labelResult.TabIndex = 3;
            this.labelResult.Text = "...";
            // 
            // FormShowResult
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(384, 767);
            this.Controls.Add(this.labelResult);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.groupBoxOffsets);
            this.Controls.Add(this.groupBoxDurations);
            this.Name = "FormShowResult";
            this.Text = "Rezultat";
            this.groupBoxDurations.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewDurations)).EndInit();
            this.groupBoxOffsets.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewOffsets)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBoxDurations;
        private System.Windows.Forms.DataGridView dataGridViewDurations;
        private System.Windows.Forms.DataGridViewTextBoxColumn lights_group_id;
        private System.Windows.Forms.DataGridViewTextBoxColumn Duration;
        private System.Windows.Forms.GroupBox groupBoxOffsets;
        private System.Windows.Forms.DataGridView dataGridViewOffsets;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label labelResult;
        private System.Windows.Forms.DataGridViewTextBoxColumn lights_system_id;
        private System.Windows.Forms.DataGridViewTextBoxColumn offset;
    }
}