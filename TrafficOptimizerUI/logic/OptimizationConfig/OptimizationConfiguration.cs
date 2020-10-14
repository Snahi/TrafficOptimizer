using System;
using System.CodeDom;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TrafficOptimizerUI.logic;

namespace TrafficOptimizerUI.logic.OptimizationConfig
{
    /// <summary>
    /// Class which stores information about the optimization to be performed. It is created 
    /// gradually by adding configure commands. When all commands are added Configure() must be
    /// called so that they have any effect. Before calling Configure() the last command can be
    /// cancelled by calling Undo().
    /// </summary>
    public class OptimizationConfiguration
    {
        // const //////////////////////////////////////////////////////////////////////////////////
        // param strings
        public const String PARAM_EVALUATOR_STR = "evaluator";
        public const String PARAM_STOP_CONDITION_STR = "stopCondition";
        public const String PARAM_END_VALUE = "endValue";
        public const String PARAM_OPTIMUM = "optimum";
        public const String PARAM_WITH_OPTIMUM = "withOptimum";
        public const String PARAM_RECORDER_STR = "recorder";
        public const String PARAM_SAVE_PATH = "savePath";
        public const String PARAM_ALGORITHM = "algorithm";
        public const String PARAM_LSHADE_NUM_OF_EVALS = "LSHADENumOfEvaluations";
        public const String PARAM_LSHADE_POP_INIT = "LSHADEPopInit";
        public const String PARAM_LSHADE_ARC_INIT = "LSHADEArcInit";
        public const String PARAM_LSHADE_HISTORY_SIZE = "LSHADEHistorySize";
        public const String PARAM_LSHADE_P = "LSHADEP";
        public const String PARAM_TIMEOUT = "timeout";
        public const String PARAM_LSHADEWGD_GENE_HIST_SIZE = "gene_hist_size";
        public const String PARAM_LSHADEWGD_GQ = "gq";
        public const String TIMEOUT_NOT_SET = "-1";
        public const int NUM_OF_LSHADE_PARAMS = 5;
        public const int LSHADE_NUM_OF_EVALS_IDX = 0;
        public const int LSHADE_POP_INIT_IDX = 1;
        public const int LSHADE_ARC_INIT_IDX = 2;
        public const int LSHADE_HISTORY_SIZE_IDX = 3;
        public const int LSHADE_P_IDX = 4;
        public const int LSHADE_DEF_PARAM_VALUE = -1;
        // string param values
        public const String SIMPLIFIED_MODEL_STRING = "SimplifiedModel";
        public const String ADVANCED_MODEL_STRING = "AdvancedModel";
        public const String ADDITIONAL_TIME_EVALUATOR_STR = "AdditionalTimeEvaluator";
        public const String AVG_TIME_EVALUATOR_STR = "AvgTimeEvaluator";
        public const String NUM_OF_EVALUATIONS_SC_STR = "NumOfEvaluations";
        public const String TIME_SC_STR = "Time";
        public const String RECORDER_RESULT_ONLY_STR = "RecResOnly";
        public const String RECORDER_RESULT_AND_STATS_STR = "RecResAndStats";
        public const String ALGORITHM_LSHADE = "LSHADE";
        public const String ALGORITHM_LSHADEWGD = "LSHADEWGD";



        // fields /////////////////////////////////////////////////////////////////////////////////
        public Dictionary<String, String> Parameters { get; }
        private Stack<ConfigureCommand> Commands;



        // constructors ///////////////////////////////////////////////////////////////////////////
        public OptimizationConfiguration()
        {
            this.Parameters = new Dictionary<String, String>();
            this.Commands = new Stack<ConfigureCommand>();
        }
        


        // methods ////////////////////////////////////////////////////////////////////////////////
        public void AddConfigureCommand(ConfigureCommand command)
        {
            Commands.Push(command);
        }



        public void Undo()
        {
            Commands.Pop();
        }



        public void Configure()
        {
            while (Commands.Count() > 0)
            {
                Commands.Pop().Execute();
            }
        }



        public void SetParameter(String parameter, String value)
        {
            Parameters[parameter] = value;
        }



        public string GetParameter(String parameter)
        {
            return Parameters[parameter];
        }



        public string GetParameter(String parameter, String defaultValue)
        {
            String res;
            bool found = Parameters.TryGetValue(parameter, out res);
            if (found)
                return res;
            else
                return defaultValue;
        }
    }
}
