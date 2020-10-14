using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TrafficOptimizerUI.logic.OptimizationConfig
{
    /// <summary>
    /// Implementation of ConfigureCommand which stores parameter-value pairs and when Execute()
    /// is called it sets the parameters in the specified OptimizationConfiguration.
    /// </summary>
    class SetParametersCommand : ConfigureCommand
    {
        // fields /////////////////////////////////////////////////////////////////////////////////
        OptimizationConfiguration Config;
        Dictionary<String, String> Parameters;



        // constructors ///////////////////////////////////////////////////////////////////////////
        public SetParametersCommand(OptimizationConfiguration config)
        {
            this.Config = config;
            Parameters = new Dictionary<String, String>();
        }



        // methods ////////////////////////////////////////////////////////////////////////////////
        public void Execute()
        {
            foreach (var param in Parameters)
            {
                Config.SetParameter(param.Key, param.Value);
            }
        }



        public void SetParameter(String paramName, String paramValue)
        {
            Parameters[paramName] = paramValue;
        }
    }
}
