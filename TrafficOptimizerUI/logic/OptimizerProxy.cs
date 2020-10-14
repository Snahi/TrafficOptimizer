using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TrafficOptimizerUI.logic.OptimizationConfig;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Forms;

namespace TrafficOptimizerUI.logic
{
    public enum ModelType
    {
        SimplifiedModel,
        AdvancedModel
    }



    /// <summary>
    /// Class which is responsible for communicating with the optimizer backend written as C++ DLL.
    /// It is an intermediary between UI and backend.
    /// </summary>
    public class OptimizerProxy
    {
        // const //////////////////////////////////////////////////////////////////////////////////
        public const String TRAFFIC_OPTIMIZER_DLL_PATH = "TrafficOptimizerDLL.dll";
        public const int LOAD_SUCCESS = 0;
        public const int LOAD_NOT_APPLICABLE = -1;
        // load simplified model errors
        public const int SIMPLIFIED_LOAD_ERR_INVALID_FILE = 1;
        public const int SIMPLIFIED_LOAD_ERR_MISSING_ATTRIBUTE = 2;
        public const int SIMPLIFIED_LOAD_ERR_WRONG_ATTRIBUTE = 3;
        public const int SIMPLIFIED_LOAD_ERR_NON_UNQ_ID = 4;
        public const int SIMPLIFIED_LOAD_ERR_GREEN_AND_RED_NOT_SPECIFIED = 5;
        public const int SIMPLIFIED_LOAD_ERR_SECTION_NOT_SPECIFIED = 6;
        public const int SIMPLIFIED_LOAD_ERR_NOT_ALL_LIGHTS_IN_LIGHTS_GROUP = 7;
        public const int SIMPLIFIED_LOAD_ERR_LIGHTS_IN_GREEN_AND_RED = 8;
        public const int SIMPLIFIED_LOAD_ERR_NO_SUCH_LIGHTS = 9;
        public const int SIMPLIFIED_LOAD_ERR_REFERENCED_ITEM_DOES_NOT_EXIST = 10;
        public const int SIMPLIFIED_LOAD_ERR_EMPTY_LIST = 11;
        public const int SIMPLIFIED_LOAD_ERR_NOT_ALL_ROUTES_ASSOCIATIED_WITH_GENERATOR = 12;
        public const int SIMPLIFIED_LOAD_ERR_INVALID_ROUTE = 13;
        public const int SIMPLIFIED_LOAD_ERR_UNCOVERED_ROADS = 14;
        public const int SIMPLIFIED_LOAD_ERR_LIGHTS_FROM_DIFFERENT_SYSTEM = 15;
        public const int SIMPLIFIED_LOAD_ERR_THE_SAME_ROAD_REFERENCED_TWICE = 16;



        // TrafficOptimizer dll ///////////////////////////////////////////////////////////////////
        /// <summary>
        /// Loads a model from the specified file.
        /// </summary>
        /// <param name="path">Path to the model file</param>
        /// <param name="modelType">model type: simplified or advanced</param>
        /// <param name="result">result of loading the model</param>
        /// <param name="errLine">number of line in which an error occurred if applicable</param>
        /// <returns>Id generated for a new optimization which concerns the newly created model
        /// </returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern void loadModel(String path, String modelType, out int result, 
            out int errLine);

        /// <summary>
        /// Once a model is loaded other parameters (e.g. evaluator, stop condition...) must
        /// be set for the optimization. It is done by calling this method.
        /// </summary>
        /// <param name="evaluatorStr">Evaluator name</param>
        /// <param name="stopConditionStr">Stop condition name</param>
        /// <param name="endValue">Value at which the stop condition should finish</param>
        /// <param name="withOptimum">Indicates whether optimal value should be taken into 
        /// account</param>
        /// <param name="optimum">Optimal value</param>
        /// <param name="recorderStr">Recorder name</param>
        /// <param name="savePath">Path to the file in which the result will be stored</param>
        /// <returns>True if success, false if fail</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool prepareOptimization(String evaluatorStr, double timeoutSeconds,
            String stopConditionStr, double endValue, bool withOptimum, double optimum,
            String recorderStr, String savePath);

        /// <summary>
        /// After other parameters of the optimizations are set (by calling prepareOptimization(.))
        /// parameters of the chosen algorithm must be set. If the chosen algorithm was LSHADE
        /// this function must be called after prepareOptimization().
        /// </summary>
        /// <param name="maxNumOfEvaluations">How many optimizations till decreasing the population
        /// size to 4</param>
        /// <param name="rInitPop"></param>
        /// <param name="rInitArc"></param>
        /// <param name="p"></param>
        /// <param name="historySize"></param>
        /// <returns>True if success, false if fail</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool prepareLSHADE(int maxNumOfEvaluations, 
            double rInitPop, double rInitArc, double p, int historySize);

        /// <summary>
        /// After other parameters of the optimizations are set (by calling prepareOptimization(.))
        /// parameters of the chosen algorithm must be set. If the chosen algorithm was LSHADEEP
        /// this function must be called after prepareOptimization().
        /// </summary>
        /// <param name="maxNumOfEvaluations">How many optimizations till decreasing the population
        /// size to 4</param>
        /// <param name="rInitPop"></param>
        /// <param name="rInitArc"></param>
        /// <param name="historySize"></param>
        /// <returns>True if success, false if fail</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool prepareLSHADEWGD(int maxNumOfEvaluations,
            double rInitPop, double rInitArc, double p, int historySize, int geneHistorySize,
            double gqImportance);

        /// <summary>
        /// After the optimization is initialized it may be started by calling this method.
        /// </summary>
        /// <returns>True if success, false if fail</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool runOptimization();

        /// <summary>
        /// Returns progress of the specified optimization
        /// </summary>
        /// <param name="pSuccess">true if success, false if some error occurred</param>
        /// <returns>Value from 0 to 1 representing progress (0 -> no progress, 1 -> finished)
        /// </returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern double getProgress(out bool pSuccess);

        /// <summary>
        /// Returns names of algorithm parameters which vary and may be displayed on a graph.
        /// </summary>
        /// <param name="pResult">Array with parameters' names</param>
        /// <returns>True if success, false if fail</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool getVariableParamsNames(
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_BSTR)] 
            out String[] pResult);

        /// <summary>
        /// Returns values for the specified parameter which occurred since the last call
        /// to this method.
        /// </summary>
        /// <param name="paramName"></param>
        /// <param name="pResult">values</param>
        /// <returns>True if success, false if fail</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool getParamValues(String paramName,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_R8)] 
            out double[] pResult);

        /// <summary>
        /// Returns fitness of the current best solution
        /// </summary>
        /// <param name="pSuccess">true if success, false if some error occurred</param>
        /// <returns>the best fitness</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern double getBestFitness(out bool pSuccess);

        /// <summary>
        /// Checks if the optimization is finished.
        /// </summary>
        /// <param name="pSuccess">true if success, false if some error occurred</param>
        /// <returns>True if the optimization is finished, false otherwise</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool isFinished(out bool pSuccess);

        /// <summary>
        /// Returns error code which is result of the optimization which was run before.
        /// </summary>
        /// <param name="pSuccess">true if success, false if some error occurred</param>
        /// <returns>Optimization run error code</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int getErrCode(out bool pSuccess);

        /// <summary>
        /// Deletes the specified optimization. Before calling it the optimization must be
        /// finished / stopped.
        /// </summary>
        /// <returns>True if success, false if some error occurred</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool deleteOptimization();

        /// <summary>
        /// Returns optimization result - durations for lights groups and offsets for lights 
        /// systems
        /// </summary>
        /// <param name="pGroupsIds"></param>
        /// <param name="pDurations"></param>
        /// <param name="pSystemsIds"></param>
        /// <param name="pOffsets"></param>
        /// <returns>True if success, false if some error occurred</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool getResult(
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_INT)] 
            out int[] pGroupsIds,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_R8)]
            out double[] pDurations,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_INT)]
            out int[] pSystemsIds,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_R8)]
            out double[] pOffsets);

        /// <summary>
        /// Returns result of optimization which was previously stored in the specified file.
        /// </summary>
        /// <param name="path"></param>
        /// <param name="pGroupsIds"></param>
        /// <param name="pDurations"></param>
        /// <param name="pSystemsIds"></param>
        /// <param name="pOffsets"></param>
        /// <param name="bestFitness"></param>
        /// <returns>True if success, false if some error occurred</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool readResult(String path,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_INT)]
            out int[] pGroupsIds,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_R8)]
            out double[] pDurations,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_INT)]
            out int[] pSystemsIds,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_R8)]
            out double[] pOffsets,
            out double bestFitness);

        /// <summary>
        /// Stops the specified optimization and saves it at the stage at which it was finished
        /// as if it was finished naturally.
        /// </summary>
        /// <returns>True if success, false if some error occurred</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool stop();

        /// <summary>
        /// Saves the best result of the specified optimization at the moment of calling this
        /// function. It doesn't stop the optimization as in the case of stop(...)
        /// </summary>
        /// <param name="path">Path to the file in which the result will be stored</param>
        /// <returns>True if success, false if some error occurred</returns>
        [DllImport(TRAFFIC_OPTIMIZER_DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool saveCurrResult(String path);



        // methods ////////////////////////////////////////////////////////////////////////////////

        // Load model /////////////////////////////////////////////////////////////////////////////
        public bool LoadModel(String path, ModelType modelType, out String errorMessage)
        {
            int result = LOAD_NOT_APPLICABLE;
            int errLine = LOAD_NOT_APPLICABLE;
            loadModel(path, ModelTypeToString(modelType), out result, out errLine);
            errorMessage = LoadResultToMessage(modelType, result, errLine);

            return result == LOAD_SUCCESS;
        }



        private String ModelTypeToString(ModelType modelType)
        {
            switch(modelType)
            {
                case ModelType.SimplifiedModel: 
                    return OptimizationConfiguration.SIMPLIFIED_MODEL_STRING;
                case ModelType.AdvancedModel:
                    return OptimizationConfiguration.ADVANCED_MODEL_STRING;
            }

            return "";
        }



        private String LoadResultToMessage(ModelType modelType, int loadResult, int errLine)
        {
            switch (modelType)
            {
                case ModelType.SimplifiedModel: 
                    return LoadResultToMessageSimplifiedModel(loadResult, errLine);
                case ModelType.AdvancedModel:
                    return LoadResultToMessageAdvancedModel(loadResult, errLine);
                default:
                    return Properties.Resources.UnknownError;
            }
        }



        private String LoadResultToMessageSimplifiedModel(int loadResult, int errLine)
        {
            if (loadResult == LOAD_SUCCESS)
                return "";

            StringBuilder msg = new StringBuilder();

            switch (loadResult)
            {
                case SIMPLIFIED_LOAD_ERR_INVALID_FILE: 
                    msg.Append(Properties.Resources.InvalidFile); break;
                case SIMPLIFIED_LOAD_ERR_MISSING_ATTRIBUTE:
                    msg.Append(Properties.Resources.MissingAttribute); break;
                case SIMPLIFIED_LOAD_ERR_WRONG_ATTRIBUTE:
                    msg.Append(Properties.Resources.WrongAttribute); break;
                case SIMPLIFIED_LOAD_ERR_NON_UNQ_ID:
                    msg.Append(Properties.Resources.NonUnqId); break;
                case SIMPLIFIED_LOAD_ERR_GREEN_AND_RED_NOT_SPECIFIED:
                    msg.Append(Properties.Resources.GreenAndRedNotSpecified); break;
                case SIMPLIFIED_LOAD_ERR_SECTION_NOT_SPECIFIED:
                    msg.Append(Properties.Resources.SectionNotSpecified); break;
                case SIMPLIFIED_LOAD_ERR_NOT_ALL_LIGHTS_IN_LIGHTS_GROUP:
                    msg.Append(Properties.Resources.NotAllLightsInGroup); break;
                case SIMPLIFIED_LOAD_ERR_LIGHTS_IN_GREEN_AND_RED:
                    msg.Append(Properties.Resources.LightsInGreenAndRed); break;
                case SIMPLIFIED_LOAD_ERR_NO_SUCH_LIGHTS:
                    msg.Append(Properties.Resources.NoSuchLights); break;
                case SIMPLIFIED_LOAD_ERR_REFERENCED_ITEM_DOES_NOT_EXIST:
                    msg.Append(Properties.Resources.NoSuchReferencedItem); break;
                case SIMPLIFIED_LOAD_ERR_EMPTY_LIST:
                    msg.Append(Properties.Resources.EmptyList); break;
                case SIMPLIFIED_LOAD_ERR_NOT_ALL_ROUTES_ASSOCIATIED_WITH_GENERATOR:
                    msg.Append(Properties.Resources.RouteWithNoGenerator); break;
                case SIMPLIFIED_LOAD_ERR_INVALID_ROUTE:
                    msg.Append(Properties.Resources.InvalidRoute); break;
                case SIMPLIFIED_LOAD_ERR_UNCOVERED_ROADS:
                    msg.Append(Properties.Resources.UncoveredRoads); break;
                case SIMPLIFIED_LOAD_ERR_LIGHTS_FROM_DIFFERENT_SYSTEM:
                    msg.Append(Properties.Resources.LightsFromOtherSystem); break;
                case SIMPLIFIED_LOAD_ERR_THE_SAME_ROAD_REFERENCED_TWICE:
                    msg.Append(Properties.Resources.RoadReferencedTwice); break;
                default:
                    msg.Append(Properties.Resources.UnknownError); break;
            }

            // append error line number
            if (errLine != LOAD_NOT_APPLICABLE)
            {
                msg.Append("\n");
                msg.Append(Properties.Resources.ErrorLine);
                msg.Append(" ");
                msg.Append(errLine);
            }

            return msg.ToString();
        }



        private String LoadResultToMessageAdvancedModel(int loadResult, int errLine)
        {
            String msg = Properties.Resources.ErrorCode + " " + loadResult + "\n" +
                Properties.Resources.ErrorLine + " " + errLine;

            return msg;
        }



        // PrepareOptimization ////////////////////////////////////////////////////////////////////



        public bool PrepareOptimization(OptimizationConfiguration config)
        {
            String evalStr = config.GetParameter(OptimizationConfiguration.PARAM_EVALUATOR_STR);
            String sCStr = config.GetParameter(OptimizationConfiguration.PARAM_STOP_CONDITION_STR);
            double endValue = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_END_VALUE));
            bool withOpti = bool.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_WITH_OPTIMUM));
            double opti = 0.0;
            if (withOpti)
                opti = double.Parse(config.GetParameter(OptimizationConfiguration.PARAM_OPTIMUM));
            String recStr = config.GetParameter(OptimizationConfiguration.PARAM_RECORDER_STR);
            String savePath = config.GetParameter(OptimizationConfiguration.PARAM_SAVE_PATH);
            String algStr = config.GetParameter(OptimizationConfiguration.PARAM_ALGORITHM);
            double timeoutSeconds = double.Parse(config.GetParameter(OptimizationConfiguration.PARAM_TIMEOUT, 
                OptimizationConfiguration.TIMEOUT_NOT_SET));

            bool genPrepRes = prepareOptimization(evalStr, timeoutSeconds, sCStr, endValue, withOpti,
                opti, recStr, savePath);
            bool prepAlgRes = false;
            if (genPrepRes)
            {
                prepAlgRes = PrepareAlgorithm(algStr, config);
            }

            return prepAlgRes;
        }



        private bool PrepareAlgorithm(String algorithmStr, OptimizationConfiguration config)
        {
            if (algorithmStr.Equals(OptimizationConfiguration.ALGORITHM_LSHADE))
                return PrepareLSHADE(config);
            else if (algorithmStr.Equals(OptimizationConfiguration.ALGORITHM_LSHADEWGD))
                return PrepareLSHADEWGD(config);
            else
                return false;
        }



        private bool PrepareLSHADE(OptimizationConfiguration config)
        {
            String defVal = OptimizationConfiguration.LSHADE_DEF_PARAM_VALUE.ToString();

            int numOfEvals = int.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_NUM_OF_EVALS, defVal));
            double rPop = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_POP_INIT, defVal));
            double rArc = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_ARC_INIT, defVal));
            double p = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_P, defVal));
            int historySize = int.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_HISTORY_SIZE, defVal));

            return prepareLSHADE(numOfEvals, rPop, rArc, p, historySize);
        }



        private bool PrepareLSHADEWGD(OptimizationConfiguration config)
        {
            String defVal = OptimizationConfiguration.LSHADE_DEF_PARAM_VALUE.ToString();

            int numOfEvals = int.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_NUM_OF_EVALS, defVal));
            double rPop = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_POP_INIT, defVal));
            double rArc = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_ARC_INIT, defVal));
            int historySize = int.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_HISTORY_SIZE, defVal));
            double p = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADE_P, defVal));
            int geneHistSiz = int.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADEWGD_GENE_HIST_SIZE, defVal));
            double gq = double.Parse(config.GetParameter(
                OptimizationConfiguration.PARAM_LSHADEWGD_GQ, defVal));

            return prepareLSHADEWGD(numOfEvals, rPop, rArc, p, historySize, geneHistSiz, gq);
        }



        // runOptimization ////////////////////////////////////////////////////////////////////////



        public bool RunOptimization()
        {
            return runOptimization();
        }



        // get optimization state /////////////////////////////////////////////////////////////////



        public String[] GetVariableParametersNames()
        {
            String[] paramsNames;
            if (getVariableParamsNames(out paramsNames))
            {
                return paramsNames;
            }
            else
                return new string[0];
        }



        public double[] GetNewParamValues(String paramName)
        {
            double[] values;
            if (getParamValues(paramName, out values))
            {
                return values;
            }
            else
                return new double[0];
        }



        /// <summary>
        /// 
        /// </summary>
        /// <returns>
        /// Value from 0 to 1, 0 -> no progress, 1 -> finished
        /// </returns>
        public double GetProgress(out bool success)
        {
            return getProgress(out success);
        }



        public double GetBestFitness(out bool success)
        {
            return getBestFitness(out success);
        }



        // save current result ////////////////////////////////////////////////////////////////////



        public bool SaveCurrResult(String path)
        {
            return saveCurrResult(path);
        }



        // stop ///////////////////////////////////////////////////////////////////////////////////



        public bool Stop()
        {
            return stop();
        }



        // finish /////////////////////////////////////////////////////////////////////////////////



        public bool IsFinished(out bool success)
        {
            bool finished = isFinished(out success);
            return finished;
        }



        public int GetErrCode(out bool success)
        {
            int errCode = getErrCode(out success);
            return errCode;
        }



        public bool DeleteOptimization()
        {
            return deleteOptimization();
        }



        // get result /////////////////////////////////////////////////////////////////////////////



        public OptimizationResult GetResult()
        {
            int[] groupsIds, systemsIds;
            double[] durations, offsets;

            if (getResult(out groupsIds, out durations, out systemsIds,
                out offsets))
            {
                OptimizationResult res = new OptimizationResult();

                int numOfGroups = groupsIds.Length;
                for (int i = 0; i < numOfGroups; ++i)
                    res.AddDuration(groupsIds[i], durations[i]);

                int numOfSystems = systemsIds.Length;
                for (int i = 0; i < numOfSystems; ++i)
                    res.AddOffset(systemsIds[i], offsets[i]);
                /*  will be ignored anyway, if mutex error occurred the best I can do is assing 
                 *  a default value, which is the value returned in case of an error*/
                bool bestSuccess;
                res.BestFitness = getBestFitness(out bestSuccess);

                return res;
            }
            else
                return null;  
        }



        public OptimizationResult ReadResult(String path)
        {
            int[] groupsIds, systemsIds;
            double[] durations, offsets;
            double bestFitness;

            if (readResult(path, out groupsIds, out durations, out systemsIds,
                out offsets, out bestFitness))
            {
                OptimizationResult res = new OptimizationResult();

                int numOfGroups = groupsIds.Length;
                for (int i = 0; i < numOfGroups; ++i)
                    res.AddDuration(groupsIds[i], durations[i]);

                int numOfSystems = systemsIds.Length;
                for (int i = 0; i < numOfSystems; ++i)
                    res.AddOffset(systemsIds[i], offsets[i]);

                res.BestFitness = bestFitness;

                return res;
            }
            else
                return null;
        }
    }
}
