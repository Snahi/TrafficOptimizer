using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TrafficOptimizerUI.logic.OptimizationConfig
{
    /// <summary>
    /// Interface for commands used during configuring an optimization run. Commands are added
    /// to OptimizationConfiguration object and when everything is finished they are executed.
    /// </summary>
    public interface ConfigureCommand
    {
        void Execute();
    }
}
