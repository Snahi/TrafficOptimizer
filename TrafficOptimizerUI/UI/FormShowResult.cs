using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using TrafficOptimizerUI.logic;

namespace TrafficOptimizerUI.UI
{
    /// <summary>
    /// Form for displaying optimization result in a table format
    /// </summary>
    public partial class FormShowResult : Form
    {
        public FormShowResult(OptimizationResult result, String modelName)
        {
            InitializeComponent();

            // set indicative name for the form (which model does it concern)
            this.Text = Properties.Resources.Result + modelName;

            // sort by group id
            SortedDictionary<int, double> durations = 
                new SortedDictionary<int, double>(result.Durations);

            // add lights groups durations
            foreach (var duration in durations)
            {
                dataGridViewDurations.Rows.Add(new object[] { duration.Key, 
                    Math.Round(duration.Value, 2) });
            }

            // sort by lights system id
            SortedDictionary<int, double> offsets = 
                new SortedDictionary<int, double>(result.Offsets);

            // add lights systems offsets
            foreach (var offset in offsets)
            {
                dataGridViewOffsets.Rows.Add(new object[] { offset.Key, 
                    Math.Round(offset.Value, 2) });
            }

            // add quality of the best solution
            labelResult.Text = result.BestFitness.ToString();
        }
    }
}
