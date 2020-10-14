using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TrafficOptimizerUI.logic
{
    /// <summary>
    /// Class for representing result of optimization. It can be displayed in FormShowResult.
    /// </summary>
    public class OptimizationResult
    {
        // fields /////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// group id - group duration
        /// </summary>
        public Dictionary<int, double> Durations { get; set; }
        /// <summary>
        /// lights system id - lights system offset/phase
        /// </summary>
        public Dictionary<int, double> Offsets { get; set; }
        /// <summary>
        /// Quality of the best solution
        /// </summary>
        public double BestFitness { get; set; }

        // constructors ///////////////////////////////////////////////////////////////////////////
        public OptimizationResult()
        {
            this.Durations = new Dictionary<int, double>();
            this.Offsets = new Dictionary<int, double>();
            this.BestFitness = double.MaxValue;
        }



        // methods ////////////////////////////////////////////////////////////////////////////////
        public void AddDuration(int id, double value)
        {
            Durations[id] = value;
        }



        public void AddOffset(int id, double value)
        {
            Offsets[id] = value;
        }
    }
}
