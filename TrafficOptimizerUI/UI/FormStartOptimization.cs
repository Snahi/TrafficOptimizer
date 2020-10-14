using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Resources;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using TrafficOptimizerUI.logic;
using TrafficOptimizerUI.logic.OptimizationConfig;
using TrafficOptimizerUI.Properties;

namespace TrafficOptimizerUI
{
    /// <summary>
    /// Form for preparing a new optimization run. It consists of several panels, each of which
    /// concerns a small part of model configuration - model selection, evaluator, stop condition,
    /// recorder and the model itself.
    /// </summary>
    public partial class FormStartOptimization : Form
    {
        // const //////////////////////////////////////////////////////////////////////////////////
        public const String DISPLAY_MEMBER_STR = "Item2";



        // fields /////////////////////////////////////////////////////////////////////////////////
        private OptimizationFinishListener FinishListener;
        private List<Tuple<ModelType, String>> ModelTypesDS;
        private List<Tuple<String, String>> EvaluatorTypesDS;
        private List<Tuple<String, String>> StopConditionTypesDS;
        private List<Tuple<String, String>> RecorderTypeDS;
        private List<Tuple<String, String>> AlgorithmDS;
        /// <summary>
        /// Configuration which is built gradually as user proceeds through the form.
        /// </summary>
        private OptimizationConfiguration Config;
        /// <summary>
        /// Object thorugh which all communication with the backend optimizer is performed.
        /// </summary>
        private OptimizerProxy Optimizer;
        /// <summary>
        /// Name of the loaded model. It is set after successully loading a model and then it is
        /// used as a parameter for FormOptimization.
        /// </summary>
        private String ModelName;



        // constructors ///////////////////////////////////////////////////////////////////////////

        public FormStartOptimization(OptimizationFinishListener finishListener)
        {
            InitializeComponent();
            InitDataSources();
            setDataSources();

            this.FinishListener = finishListener;

            // initialize fields
            this.Config = new OptimizationConfiguration();
            this.Optimizer = new OptimizerProxy();

            // set default values
            // model
            this.listBoxModelType.SelectedIndex = 0;
            this.buttonGoToEvaluator.Enabled = false;
            // evaluator
            this.listBoxEvaluator.SelectedIndex = 0;
            // stop condition
            this.listBoxStopConditions.SelectedIndex = 0;
            buttonGoToRecorder.Enabled = false;
            // recorder
            this.listBoxRecorderType.SelectedIndex = 0;
            buttonGoToAlgorithm.Enabled = false;
            // algorithm
            this.listBoxAlgorithm.SelectedIndex = 0;
        }



        private void InitDataSources()
        {
            initModelTypeDS();
            initEvaluatorTypesDS();
            initStopConditionsDS();
            initRecorderTypeDS();
            initAlgorithmDS();
        }



        private void initModelTypeDS()
        {
            ModelTypesDS = new List<Tuple<ModelType, String>>();
            String simplifiedModelDisp = Properties.Resources.SimplifiedModel;
            String advancedModelDisp = Properties.Resources.AdvancedModel;
            ModelTypesDS.Add(new Tuple<ModelType, String>(ModelType.SimplifiedModel, simplifiedModelDisp));
            ModelTypesDS.Add(new Tuple<ModelType, String>(ModelType.AdvancedModel, advancedModelDisp));
        }



        private void initEvaluatorTypesDS()
        {
            EvaluatorTypesDS = new List<Tuple<String, String>>();
            EvaluatorTypesDS.Add(new Tuple<String, String>(
                OptimizationConfiguration.ADDITIONAL_TIME_EVALUATOR_STR,
                Properties.Resources.AdditionalTimeEvaluator));
            EvaluatorTypesDS.Add(new Tuple<String, String>(
                OptimizationConfiguration.AVG_TIME_EVALUATOR_STR,
                Properties.Resources.AvgTimeEvaluator));
        }



        private void initStopConditionsDS()
        {
            StopConditionTypesDS = new List<Tuple<String, String>>();
            StopConditionTypesDS.Add(new Tuple<String, String>(
                OptimizationConfiguration.NUM_OF_EVALUATIONS_SC_STR,
                Properties.Resources.NumOfEvaluations));
            StopConditionTypesDS.Add(new Tuple<String, String>(
                OptimizationConfiguration.TIME_SC_STR,
                Properties.Resources.TimeSeconds));
        }



        private void initRecorderTypeDS()
        {
            RecorderTypeDS = new List<Tuple<String, String>>();
            RecorderTypeDS.Add(new Tuple<string, string>(
                OptimizationConfiguration.RECORDER_RESULT_ONLY_STR,
                Properties.Resources.RecorderResultOnly));
            RecorderTypeDS.Add(new Tuple<string, string>(
                OptimizationConfiguration.RECORDER_RESULT_AND_STATS_STR,
                Properties.Resources.RecorderResultAndStats));
        }



        private void initAlgorithmDS()
        {
            AlgorithmDS = new List<Tuple<String, String>>();
            AlgorithmDS.Add(new Tuple<string, string>(
                OptimizationConfiguration.ALGORITHM_LSHADE,
                Properties.Resources.LSHADE));
            AlgorithmDS.Add(new Tuple<string, string>(
                OptimizationConfiguration.ALGORITHM_LSHADEWGD,
                Properties.Resources.LSHADEWGD));
        }



        private void setDataSources()
        {
            // model type
            listBoxModelType.DataSource = ModelTypesDS;
            listBoxModelType.DisplayMember = DISPLAY_MEMBER_STR;
            // evaluator type
            listBoxEvaluator.DataSource = EvaluatorTypesDS;
            listBoxEvaluator.DisplayMember = DISPLAY_MEMBER_STR;
            // stop condition
            listBoxStopConditions.DataSource = StopConditionTypesDS;
            listBoxStopConditions.DisplayMember = DISPLAY_MEMBER_STR;
            // recorder
            listBoxRecorderType.DataSource = RecorderTypeDS;
            listBoxRecorderType.DisplayMember = DISPLAY_MEMBER_STR;
            // algorithm
            listBoxAlgorithm.DataSource = AlgorithmDS;
            listBoxAlgorithm.DisplayMember = DISPLAY_MEMBER_STR;
        }



        // event handlers /////////////////////////////////////////////////////////////////////////



        private void listBoxModelType_SelectedIndexChanged(object sender, EventArgs e)
        {
            textBoxFilePath.Text = "";
            buttonGoToEvaluator.Enabled = false;
        }



        private void buttonChooseModel_Click(object sender, EventArgs e)
        {
            errorProviderModel.Clear();
            openFileDialogModel.FileName = null;
            openFileDialogModel.ShowDialog();
            String path = openFileDialogModel.FileName;
            // display the path in a text box
            textBoxFilePath.Text = path;

            if (!String.IsNullOrWhiteSpace(path)) // if user specified the file
            {
                String errorMessage = "";
                (ModelType modelType, _) = listBoxModelType.SelectedItem as Tuple<ModelType, String>;
                buttonChooseModel.Enabled = false;
                labelModelLoading.Visible = true;
                bool loadRes = Optimizer.LoadModel(path, modelType, out errorMessage);
                if (loadRes) // successfully loaded the model
                {
                    String[] pathSplitted = path.Split(Path.DirectorySeparatorChar);
                    // if success pathSplitted must have at least one element, the last one is the file name
                    ModelName = pathSplitted[pathSplitted.Length - 1];
                    buttonGoToEvaluator.Enabled = true;
                }
                else // wrong model
                {
                    errorProviderModel.SetError(textBoxFilePath, errorMessage);
                    buttonGoToEvaluator.Enabled = false;
                }
                buttonChooseModel.Enabled = true;
                labelModelLoading.Visible = false;
            }
            else
                buttonGoToEvaluator.Enabled = false;
        }



        private void buttonGoToEvaluator_Click(object sender, EventArgs e)
        {
            panelSelectModel.Hide();
            panelSelectEvaluator.Show();
        }



        private void textBoxTimeout_TextChanged(object sender, EventArgs e)
        {
            String timeoutStr = textBoxTimeout.Text;
            if (!String.IsNullOrEmpty(timeoutStr))
            {
                double timeout;
                if (double.TryParse(timeoutStr, out timeout) && timeout > 0)
                {
                    buttonGoToStopCondition.Enabled = true;
                    errorProviderTimeout.Clear();
                }
                else
                {
                    errorProviderTimeout.SetError(textBoxTimeout,
                        Properties.Resources.ErrWrongValue);
                    buttonGoToStopCondition.Enabled = false;
                }
            }
            else
            {
                errorProviderTimeout.Clear();
                buttonGoToStopCondition.Enabled = true;
            }
        }



        private void buttonGoToStopCondition_Click(object sender, EventArgs e)
        {
            (String evaluatorType,_) = listBoxEvaluator.SelectedItem as Tuple<String,String>;
            SetParametersCommand command = new SetParametersCommand(Config);
            command.SetParameter(OptimizationConfiguration.PARAM_EVALUATOR_STR, evaluatorType);
            if (!String.IsNullOrEmpty(textBoxTimeout.Text))
                command.SetParameter(OptimizationConfiguration.PARAM_TIMEOUT, textBoxTimeout.Text);

            Config.AddConfigureCommand(command);
            panelSelectEvaluator.Visible = false;
            panelSelectStopCondition.Visible = true;
        }



        private void buttonBackToModel_Click(object sender, EventArgs e)
        {
            // do not undo config, because nothing was changed so far
            panelSelectEvaluator.Visible = false;
            panelSelectModel.Visible = true;
        }



        private void buttonGoToRecorder_Click(object sender, EventArgs e)
        {
            (String scType,_) = listBoxStopConditions.SelectedItem as Tuple<String, String>;
            
            // check if fields are valid
            if (String.IsNullOrEmpty(errorProviderEndValue.GetError(textBoxEndValue)) &&
                String.IsNullOrEmpty(errorProviderOptimum.GetError(textBoxOptimum)))
            {
                String endValueStr = textBoxEndValue.Text;
                String optimumStr = textBoxOptimum.Text;
                SetParametersCommand command = new SetParametersCommand(Config);
                
                // set parameters
                // stop condition type
                command.SetParameter(OptimizationConfiguration.PARAM_STOP_CONDITION_STR, scType);
                // end value
                command.SetParameter(OptimizationConfiguration.PARAM_END_VALUE, endValueStr);
                // optimum
                if (String.IsNullOrWhiteSpace(optimumStr)) // no optimum
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_WITH_OPTIMUM,
                        false.ToString());
                }
                else // with optimum
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_OPTIMUM, optimumStr);
                    command.SetParameter(OptimizationConfiguration.PARAM_WITH_OPTIMUM, 
                        true.ToString());
                }

                Config.AddConfigureCommand(command);
                panelSelectStopCondition.Visible = false;
                panelSelectRecorder.Visible = true;
            }
        }



        private void textBoxEndValue_TextChanged(object sender, EventArgs e)
        {
            String endValStr = textBoxEndValue.Text;
            double endVal;
            if (String.IsNullOrWhiteSpace(endValStr))
            {
                errorProviderEndValue.SetError(textBoxEndValue,
                    Properties.Resources.ErrObligatoryField);
                buttonGoToRecorder.Enabled = false;
            }
            else if (!double.TryParse(endValStr, out endVal))
            {
                errorProviderEndValue.SetError(textBoxEndValue,
                    Properties.Resources.ErrWrongValue);
                buttonGoToRecorder.Enabled = false;
            }
            else // all ok
            {
                errorProviderEndValue.Clear();
                buttonGoToRecorder.Enabled = true;
            }
        }



        private void textBoxOptimum_TextChanged(object sender, EventArgs e)
        {
            String optiStr = textBoxOptimum.Text;
            double opti;
            if (!String.IsNullOrEmpty(optiStr) && !double.TryParse(optiStr, out opti))
            {
                errorProviderOptimum.SetError(textBoxOptimum,
                    Properties.Resources.ErrWrongValue);
            }
            else // all ok
            {
                errorProviderOptimum.Clear();
            }
        }

        private void buttonBackToEvaluator_Click(object sender, EventArgs e)
        {
            Config.Undo();
            panelSelectStopCondition.Visible = false;
            panelSelectEvaluator.Visible = true;
        }



        private void buttonChooseSaveFile_Click(object sender, EventArgs e)
        {
            saveFileDialogSaveResult.ShowDialog();
            String filePath = saveFileDialogSaveResult.FileName;
            textBoxSaveFilePath.Text = filePath;

            if (String.IsNullOrWhiteSpace(filePath))
                buttonGoToAlgorithm.Enabled = false;
            else
                buttonGoToAlgorithm.Enabled = true;
        }



        private void buttonGoToAlgorithm_Click(object sender, EventArgs e)
        {
            SetParametersCommand command = new SetParametersCommand(Config);
            (String recorder, _) = listBoxRecorderType.SelectedItem as Tuple<String, String>;
            command.SetParameter(OptimizationConfiguration.PARAM_RECORDER_STR, recorder);
            command.SetParameter(OptimizationConfiguration.PARAM_SAVE_PATH, 
                textBoxSaveFilePath.Text);
            Config.AddConfigureCommand(command);

            panelSelectRecorder.Visible = false;
            panelSelectAlgorithm.Visible = true;
        }



        private void buttonBackToStopCondition_Click(object sender, EventArgs e)
        {
            Config.Undo();
            panelSelectRecorder.Visible = false;
            panelSelectStopCondition.Visible = true;
        }



        private void buttonBackToRecorder_Click(object sender, EventArgs e)
        {
            Config.Undo();
            panelSelectAlgorithm.Visible = false;
            panelSelectRecorder.Visible = true;
        }



        private void buttonGoToParameters_Click(object sender, EventArgs e)
        {
            (String algorithm,_) = listBoxAlgorithm.SelectedItem as Tuple<String, String>;
            SetParametersCommand command = new SetParametersCommand(Config);
            command.SetParameter(OptimizationConfiguration.PARAM_ALGORITHM, algorithm);
            Config.AddConfigureCommand(command);
            panelSelectAlgorithm.Visible = false;
            if (algorithm.Equals(OptimizationConfiguration.ALGORITHM_LSHADE))
            {
                panelLSHADE.Visible = true;
            }
            else if (algorithm.Equals(OptimizationConfiguration.ALGORITHM_LSHADEWGD))
            {
                panelLSHADEWGD.Visible = true;
            }
        }



        // panel LSHADE ///////////////////////////////////////////////////////////////////////////



        private void textBoxMaxEvaluations_TextChanged(object sender, EventArgs e)
        {
            int maxEvaluations = -1;
            String maxEvaluationsStr = textBoxMaxEvaluations.Text;
            if (!String.IsNullOrEmpty(maxEvaluationsStr))
            {
                if (!int.TryParse(maxEvaluationsStr, out maxEvaluations) || maxEvaluations <= 0)
                {
                    errorProviderMaxEvaluations.SetError(textBoxMaxEvaluations,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

             errorProviderMaxEvaluations.Clear();
        }



        private void textBoxPopInit_TextChanged(object sender, EventArgs e)
        {
            double popInit = -1.0;
            String popInitStr = textBoxPopInit.Text;
            if (!String.IsNullOrEmpty(popInitStr))
            {
                if (!double.TryParse(popInitStr, out popInit) || popInit <= 0)
                {
                    errorProviderPopInit.SetError(textBoxPopInit,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderPopInit.Clear();
        }



        private void textBoxArcInit_TextChanged(object sender, EventArgs e)
        {
            double arcInit = -1.0;
            String arcInitStr = textBoxArcInit.Text;
            if (!String.IsNullOrEmpty(arcInitStr))
            {
                if (!double.TryParse(arcInitStr, out arcInit) || arcInit <= 0)
                {
                    errorProviderArcInit.SetError(textBoxArcInit,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderArcInit.Clear();
        }



        private void textBoxHistorySize_TextChanged(object sender, EventArgs e)
        {
            int historySize = -1;
            String historySizeStr = textBoxHistorySize.Text;
            if (!String.IsNullOrEmpty(historySizeStr))
            {
                if (!int.TryParse(historySizeStr, out historySize) || historySize <= 0)
                {
                    errorProviderHistorySize.SetError(textBoxHistorySize,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderHistorySize.Clear();
        }



        private void textBoxP_TextChanged(object sender, EventArgs e)
        {
            double p = -1.0;
            String pStr = textBoxP.Text;
            if (!String.IsNullOrEmpty(pStr))
            {
                if (!double.TryParse(pStr, out p) || p <= 0.0 || p > 1.0)
                {
                    errorProviderP.SetError(textBoxP,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderP.Clear();
        }



        private void buttonBackToAlgorithm_Click(object sender, EventArgs e)
        {
            Config.Undo();
            panelLSHADE.Visible = false;
            panelSelectAlgorithm.Visible = true;
        }



        private void buttonStartLSHADE_Click(object sender, EventArgs e)
        {
            // if no errors
            if (String.IsNullOrEmpty(errorProviderMaxEvaluations.GetError(textBoxMaxEvaluations)) &&
                String.IsNullOrEmpty(errorProviderPopInit.GetError(textBoxPopInit)) &&
                String.IsNullOrEmpty(errorProviderArcInit.GetError(textBoxArcInit)) &&
                String.IsNullOrEmpty(errorProviderHistorySize.GetError(textBoxHistorySize)) &&
                String.IsNullOrEmpty(errorProviderP.GetError(textBoxP)))
            {
                SetParametersCommand command = new SetParametersCommand(Config);
                String maxEvalsStr = textBoxMaxEvaluations.Text;
                String popInitStr = textBoxPopInit.Text;
                String arcInitStr = textBoxArcInit.Text;
                String histSizeStr = textBoxHistorySize.Text;
                String pStr = textBoxP.Text;
                if (!String.IsNullOrEmpty(maxEvalsStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_NUM_OF_EVALS, 
                        maxEvalsStr);
                }
                if (!String.IsNullOrEmpty(popInitStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_POP_INIT, 
                        popInitStr);
                }
                if (!String.IsNullOrEmpty(arcInitStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_ARC_INIT,
                        arcInitStr);
                }
                if (!String.IsNullOrEmpty(histSizeStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_HISTORY_SIZE,
                        histSizeStr);
                }
                if (!String.IsNullOrEmpty(pStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_P, pStr);
                }

                Config.AddConfigureCommand(command);
                StartOptimization();
            } 
        }



        // panel LSHADEEP /////////////////////////////////////////////////////////////////////////



        private void textBoxMaxEvaluationsLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            int maxEvaluations = -1;
            String maxEvaluationsStr = textBoxMaxEvaluationsLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(maxEvaluationsStr))
            {
                if (!int.TryParse(maxEvaluationsStr, out maxEvaluations) || maxEvaluations <= 0)
                {
                    errorProviderMaxEvaluationsLSHADEWGD.SetError(textBoxMaxEvaluationsLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderMaxEvaluationsLSHADEWGD.Clear();
        }



        private void textBoxRpopLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            double popInit = -1.0;
            String popInitStr = textBoxRpopLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(popInitStr))
            {
                if (!double.TryParse(popInitStr, out popInit) || popInit <= 0)
                {
                    errorProviderPopInitLSHADEWGD.SetError(textBoxRpopLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderPopInitLSHADEWGD.Clear();
        }



        private void textBoxRarcLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            double arcInit = -1.0;
            String arcInitStr = textBoxRarcLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(arcInitStr))
            {
                if (!double.TryParse(arcInitStr, out arcInit) || arcInit <= 0)
                {
                    errorProviderArcInitLSHADEWGD.SetError(textBoxRarcLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderArcInitLSHADEWGD.Clear();
        }



        private void textBoxHSizeLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            int historySize = -1;
            String historySizeStr = textBoxHSizeLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(historySizeStr))
            {
                if (!int.TryParse(historySizeStr, out historySize) || historySize <= 0)
                {
                    errorProviderHistorySizeLSHADEWGD.SetError(textBoxHSizeLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderHistorySizeLSHADEWGD.Clear();
        }



        private void textBoxPLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            double p = -1.0;
            String pStr = textBoxPLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(pStr))
            {
                if (!double.TryParse(pStr, out p) || p <= 0.0 || p > 1.0)
                {
                    errorProviderPLSHADEWGD.SetError(textBoxPLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderPLSHADEWGD.Clear();
        }



        private void textBoxGeneHistLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            int histSize = -1;
            String histSizeStr = textBoxGeneHistLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(histSizeStr))
            {
                if (!int.TryParse(histSizeStr, out histSize) || histSize <= 0)
                {
                    errorProviderGeneHistLSHADEWGD.SetError(textBoxGeneHistLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderGeneHistLSHADEWGD.Clear();
        }



        private void textBoxGqImportanceLSHADEWGD_TextChanged(object sender, EventArgs e)
        {
            double gqImp = -1.0;
            String gqImpStr = textBoxGqImportanceLSHADEWGD.Text;
            if (!String.IsNullOrEmpty(gqImpStr))
            {
                if (!double.TryParse(gqImpStr, out gqImp) || gqImp <= 0.0 || gqImp > 1.0)
                {
                    errorProviderGQLSHADEWGD.SetError(textBoxGqImportanceLSHADEWGD,
                        Properties.Resources.ErrWrongValue);
                    return;
                }
            }

            errorProviderGQLSHADEWGD.Clear();
        }



        private void buttonStartLSHADEWGD_Click(object sender, EventArgs e)
        {
            // if no errors
            if (String.IsNullOrEmpty(errorProviderMaxEvaluationsLSHADEWGD.GetError(textBoxMaxEvaluationsLSHADEWGD)) &&
                String.IsNullOrEmpty(errorProviderPopInitLSHADEWGD.GetError(textBoxRpopLSHADEWGD)) &&
                String.IsNullOrEmpty(errorProviderArcInitLSHADEWGD.GetError(textBoxRarcLSHADEWGD)) &&
                String.IsNullOrEmpty(errorProviderHistorySizeLSHADEWGD.GetError(textBoxHSizeLSHADEWGD)) &&
                String.IsNullOrEmpty(errorProviderPLSHADEWGD.GetError(textBoxPLSHADEWGD)) &&
                String.IsNullOrEmpty(errorProviderGeneHistLSHADEWGD.GetError(textBoxGeneHistLSHADEWGD)) &&
                String.IsNullOrEmpty(errorProviderGQLSHADEWGD.GetError(textBoxGqImportanceLSHADEWGD)))
            {
                SetParametersCommand command = new SetParametersCommand(Config);
                String maxEvalsStr = textBoxMaxEvaluationsLSHADEWGD.Text;
                String popInitStr = textBoxRpopLSHADEWGD.Text;
                String arcInitStr = textBoxRarcLSHADEWGD.Text;
                String histSizeStr = textBoxHSizeLSHADEWGD.Text;
                String pStr = textBoxPLSHADEWGD.Text;
                String geneHistSizStr = textBoxGeneHistLSHADEWGD.Text;
                String gqStr = textBoxGqImportanceLSHADEWGD.Text;
                if (!String.IsNullOrEmpty(maxEvalsStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_NUM_OF_EVALS,
                        maxEvalsStr);
                }
                if (!String.IsNullOrEmpty(popInitStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_POP_INIT,
                        popInitStr);
                }
                if (!String.IsNullOrEmpty(arcInitStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_ARC_INIT,
                        arcInitStr);
                }
                if (!String.IsNullOrEmpty(histSizeStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_HISTORY_SIZE,
                        histSizeStr);
                }
                if (!String.IsNullOrEmpty(pStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADE_P, pStr);
                }
                if (!String.IsNullOrEmpty(geneHistSizStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADEWGD_GENE_HIST_SIZE, 
                        geneHistSizStr);
                }
                if (!String.IsNullOrEmpty(gqStr))
                {
                    command.SetParameter(OptimizationConfiguration.PARAM_LSHADEWGD_GQ,
                        gqStr);
                }

                Config.AddConfigureCommand(command);
                StartOptimization();
            }
        }



        private void buttonLSHADEWGDBackToAlgorithm_Click(object sender, EventArgs e)
        {
            Config.Undo();
            panelLSHADEWGD.Visible = false;
            panelSelectAlgorithm.Visible = true;
        }



        private void StartOptimization()
        {
            Config.Configure();
            FormOptimization optiForm = new FormOptimization(Optimizer, ModelName, FinishListener);
            optiForm.TopMost = false;
            optiForm.Show();
            bool startRes = optiForm.StartOptimization(Config);
            if (startRes)
                this.DialogResult = DialogResult.OK;
            else
                this.DialogResult = DialogResult.Cancel;
            this.Close();
        }
    }
}
