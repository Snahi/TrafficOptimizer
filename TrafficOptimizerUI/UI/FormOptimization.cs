using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows.Forms;
using TrafficOptimizerUI.logic;
using TrafficOptimizerUI.logic.OptimizationConfig;
using System.Diagnostics;
using LiveCharts;
using LiveCharts.Wpf;
using TrafficOptimizerUI.UI;

namespace TrafficOptimizerUI
{
    /// <summary>
    /// Form for displying a running optimization. It allows the user to control it (stop or save
    /// at any moment) and displays progress and parameters on graphs.
    /// </summary>
    public partial class FormOptimization : Form
    {
        // const //////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// How often displayed state is updated. In milliseconds.
        /// </summary>
        public const int STATE_UPDATE_PERIOD = 10000;
        public const String BEST_FITNESS_PARAM_NAME = "best_fitness";
        /// <summary>
        /// Opti thread will sleep to free the processor, but in order to make closing of the form
        /// responsive the sleep time should be short, rather shorter than STATE_UPDATE_PERIOD
        /// </summary>
        public const int OPTI_THREAD_SLEEP_TIME = 1000;
        public const int PROGRESS_BAR_MAXIMUM = 10000; // not 100, because then the precission would be smaller
        /// <summary>
        /// How much progress must be achieved before the remaining time is calculated. This is
        /// set so that the precision is bigger and to avoid 0 division.
        /// </summary>
        public const double MIN_PROGRESS_FOR_REM_TIME = 0.01;
        // finish err codes
        public const int ERR_CODE_NOT_SET = -1;
        public const int ERR_CODE_SUCCESS = 0;
        public const int ERR_CODE_SAVE_ERR = 1;



        // fields /////////////////////////////////////////////////////////////////////////////////
        // delegates for accessing controls from the update thread
        private delegate void SafeGraphUpdateDelegate(LiveCharts.WinForms.CartesianChart graph,
            double param);
        private delegate void SafeLabelUpdate(Label label, String newValue);
        private delegate void SafeProgressBarUpdate(double progress);
        private delegate void SafeButtonEnabled(Button button, bool enabled);
        private OptimizationFinishListener FinishListener;
        /// <summary>
        /// All communication with backend is performed through it.
        /// </summary>
        private OptimizerProxy Optimizer;
        /// <summary>
        /// thread which communicates with optimizer and changes displayed state, e.g. time left,
        /// current value of a parameter, etc. Also when the optimization is finished it stops
        /// everything and displays the result.
        /// </summary>
        private Thread OptiStateThread;
        /// <summary>
        /// flag indicating if OptiStateThread should continue. If false it finishes.
        /// </summary>
        private bool IsStateThreadRunning;
        /// <summary>
        /// Counts time since last state update. When the time is reached the state is changed and
        /// StateUpdateTimer is reset.
        /// </summary>
        private Stopwatch StateUpdateTimer;
        /// <summary>
        /// Calls to optimizer are done periodically, not whenever an iteration is performed, so it
        /// is possible that several iterations have finished in the current update period. In that
        /// case multiple values must be taken.
        /// (parameter name, parameter values)
        /// </summary>
        private Dictionary<String, List<double>> OptiParams;
        /// <summary>
        /// Names of parameters of the currently running algorithm
        /// </summary>
        String[] paramsNames;
        /// <summary>
        /// Contains name of the parameter currently displayed on cartesianChart1. 
        /// Must be accessed with MutGraph.
        /// </summary>
        private String GraphParam;
        /// <summary>
        /// must be acquired whenever updating a graph or graph label.
        /// </summary>
        private Mutex MutGraph;
        /// <summary>
        /// Timer which counts time since optimization start. It is used to calculate estimated
        /// time to the end.
        /// </summary>
        private Stopwatch OptimizationTimer;
        /// <summary>
        /// When optimization is finished OptiStateThread changes it to true, stops everything and
        /// displays the result of the optimization. MutIsFinished should be acquired before
        /// accessing it (excluding initialization).
        /// </summary>
        private bool IsFinished;
        private Mutex MutIsFinished;
        /// <summary>
        /// It is to prevent displaying result twice. It's set to true when the method finish()
        /// is called for the first time.
        /// </summary>
        private bool MethodFinishPerformed;
        private Mutex MutMethodFinshPerformed;
        /// <summary>
        /// Name (file name) of the currently optimized model. It is displayed on top of the form
        /// to avoid confusion which is the currently running optimization.
        /// </summary>
        private String ModelName;



        // constructors ///////////////////////////////////////////////////////////////////////////
        public FormOptimization(OptimizerProxy optimizer, String modelName, 
            OptimizationFinishListener finishListener)
        {
            InitializeComponent();

            this.FinishListener = finishListener;
            // turnoff all labels before the form is initialized
            comboBoxParam1.Enabled = false;
            buttonStop.Enabled = false;
            buttonSave.Enabled = false;

            // init fields
            this.IsFinished = true; // so that the window can be turned of in case of an error
            MutIsFinished = new Mutex();
            this.Optimizer = optimizer;
            this.StateUpdateTimer = new Stopwatch();
            StateUpdateTimer.Start();
            this.OptiParams = new Dictionary<String, List<Double>>();
            this.IsStateThreadRunning = true;
            this.FormClosing += FormOptimization_FormClosing;
            this.OptimizationTimer = new Stopwatch();
            MutGraph = new Mutex();
            this.ModelName = modelName;
            this.Text = Properties.Resources.CurrentOptimizationModel + ModelName;
            this.MethodFinishPerformed = false;
            this.MutMethodFinshPerformed = new Mutex();
        }



        // methods ////////////////////////////////////////////////////////////////////////////////
        private void FormOptimization_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (IsFinished)
            {
                stopOptiStateThread();
                Optimizer.DeleteOptimization();
                FinishListener.Finished();
            }
            else
            {
                // prevent quitting if the optimization is in progress
                e.Cancel = true;
                MessageBox.Show(Properties.Resources.StopOptiFirst);
            }
        }



        private void stopOptiStateThread()
        {
            IsStateThreadRunning = false;
            try
            {
                if (OptiStateThread.IsAlive)
                {
                    OptiStateThread.Join();
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show(Properties.Resources.UnknownError + "\n" + exc.Message);
            }
        }



        public bool StartOptimization(OptimizationConfiguration optiConfig)
        {
            if (Optimizer.PrepareOptimization(optiConfig))
            {
                if (Optimizer.RunOptimization())
                {
                    StartOptimizationInternally();
                    return true;
                }
                else
                {
                    MessageBox.Show(Properties.Resources.ErrRunOptimization);
                }
            }
            else // error during preparing
            {
                MessageBox.Show(Properties.Resources.ErrPrepareOptimization);
            }

            return false;
        }



        public void StartOptimizationInternally()
        {
            OptimizationTimer.Reset();
            OptimizationTimer.Start();
            LoadOptimizationCharacteristics();
            initProgressBar();
            initGraph();
            IsFinished = false; // no need to lock mutex here, the conficting thread is not yet started
            StartOptiStateThread();
            // enable controls
            comboBoxParam1.Enabled = true;
            buttonStop.Enabled = true;
        }



        private void LoadOptimizationCharacteristics()
        {
            paramsNames = Optimizer.GetVariableParametersNames();
            // initialize params
            foreach (var paramName in paramsNames)
            {
                OptiParams[paramName] = new List<double>();
            }

            // update data sources
            List<String> paramsNamesList = new List<String>(paramsNames);
            int selectedIdx = paramsNamesList.IndexOf(BEST_FITNESS_PARAM_NAME); // by default select best_fitness
            if (selectedIdx < 0)
                selectedIdx = 0;
            comboBoxParam1.DataSource = paramsNamesList;
            comboBoxParam1.SelectedIndex = selectedIdx;
            GraphParam = paramsNames[selectedIdx];
        }



        private void initGraph()
        {
            // lock mutex
            try
            {
                MutGraph.WaitOne();
            }
            catch (Exception e)
            {
                MessageBox.Show(Properties.Resources.UnknownError +
                    "\n" + e.Message);
            }

            // init
            cartesianChart1.Series = new SeriesCollection
            {
                new LineSeries
                {
                    Title = GraphParam,
                    Values = new ChartValues<double>{ },
                    LineSmoothness = 0
                }
            };

            cartesianChart1.LegendLocation = LegendLocation.Bottom;

            // unlock mutex
            try
            {
                MutGraph.ReleaseMutex();
            }
            catch (Exception e)
            {
                MessageBox.Show(Properties.Resources.UnknownError +
                    "\n" + e.Message);
            }
        }



        private void StartOptiStateThread()
        {
            IsStateThreadRunning = true;
            OptiStateThread = new Thread(updateOptimizationState);
            OptiStateThread.IsBackground = true;
            OptiStateThread.Priority = ThreadPriority.BelowNormal; // so that it's responsive on join
            OptiStateThread.Start();
        }



        private void updateOptimizationState()
        {
            bool getProgressSuccess;
            double progress;
            bool isFinishedSuccess;
            bool isFinishedLocal;
            try
            {
                MutIsFinished.WaitOne();
                isFinishedLocal = IsFinished;
                MutIsFinished.ReleaseMutex();
            }
            catch (Exception e)
            {
                MessageBox.Show(Properties.Resources.UnknownError +
                            "\n" + e.Message);
                return;
            }

            while (IsStateThreadRunning && !isFinishedLocal)
            {
                if (StateUpdateTimer.ElapsedMilliseconds >= STATE_UPDATE_PERIOD)
                {
                    // read parameters of optimizer and update graph accordingly
                    updateParams();
                    // update progress bar and estimated remaining time
                    progress = Optimizer.GetProgress(out getProgressSuccess);
                    if (progress > 0.0)
                        threadSafeEnableButton(buttonSave, true);
                    if (getProgressSuccess)
                        updateProgress(progress);
                    else
                        MessageBox.Show(Properties.Resources.CantGetProgress);

                    StateUpdateTimer.Restart();

                    if (Optimizer.IsFinished(out isFinishedSuccess))
                    {
                        if (isFinishedSuccess)
                            finish();
                        else
                            MessageBox.Show(Properties.Resources.ErrIsFinished);
                    }
                }

                Thread.Sleep(OPTI_THREAD_SLEEP_TIME); // free processor

                // copy finished to local variable
                try
                {
                    MutIsFinished.WaitOne();
                    isFinishedLocal = IsFinished;
                    MutIsFinished.ReleaseMutex();
                }
                catch (Exception e)
                {
                    MessageBox.Show(Properties.Resources.UnknownError +
                            "\n" + e.Message);
                    return;
                }
            }
        }



        bool processFinishErrCode(int errCode, out String errMsg)
        {
            switch (errCode)
            {
                case ERR_CODE_SUCCESS: errMsg = ""; return true;
                case ERR_CODE_SAVE_ERR: errMsg = Properties.Resources.ErrCodeSaveErr; return false;
                case ERR_CODE_NOT_SET: errMsg = Properties.Resources.ErrCodeNotSet; return false;
                default:
                    errMsg = Properties.Resources.ErrCodeUnexpErr;
                    return false;
            }
        }



        private void finish()
        {
            try
            {
                MutMethodFinshPerformed.WaitOne();
                if (!MethodFinishPerformed)
                {
                    updateProgressBar(1.0);
                    ThreadSafeLabelUpdate(labelOptimizing, Properties.Resources.OptimizationFinished);
                    ThreadSafeLabelUpdate(labelRemainingTime, 0.ToString());
                    buttonSave.Enabled = false;
                    buttonStop.Enabled = false;
                    bool errCodeSuccess;
                    String errMsg;
                    int errCode = Optimizer.GetErrCode(out errCodeSuccess);
                    if (errCodeSuccess)
                    {
                        try
                        {
                            MutIsFinished.WaitOne();
                            IsFinished = true;
                            MutIsFinished.ReleaseMutex();
                        }
                        catch (Exception e)
                        {
                            MessageBox.Show(Properties.Resources.UnknownError +
                                "\n" + e.Message);
                            return;
                        }
                        if (processFinishErrCode(errCode, out errMsg))
                        {
                            Form resultForm = new FormShowResult(Optimizer.GetResult(), ModelName);
                            resultForm.ShowDialog();
                        }
                        else
                        {
                            MessageBox.Show(errMsg);
                        }
                    }
                    else
                    {
                        MessageBox.Show(Properties.Resources.CantGetErrCode);
                    }

                    MethodFinishPerformed = true;
                }
            }
            catch (Exception)
            {
                MessageBox.Show(Properties.Resources.UnknownError);
            }
        }



        private void updateParams()
        {
            updateBestSolution();

            List<double> oldValues;
            double[] newValues;
            foreach (var paramName in paramsNames)
            {
                oldValues = OptiParams[paramName];    
                newValues = Optimizer.GetNewParamValues(paramName);
                foreach (var value in newValues)
                {
                    oldValues.Add(value);

                    try
                    {
                        MutGraph.WaitOne();
                        // update graph if appropriate
                        if (paramName.Equals(GraphParam))
                            updateGraphThreadSafe(cartesianChart1, value);

                        MutGraph.ReleaseMutex();
                    }
                    catch (Exception e)
                    {
                        MessageBox.Show(Properties.Resources.UnknownError +
                            "\n" + e.Message);
                    }
                }
            }
        }



        private void updateBestSolution()
        {
            bool bestSuccess = false;
            double bestFitness = Optimizer.GetBestFitness(out bestSuccess);
            if (bestSuccess)
                ThreadSafeLabelUpdate(labelBestSolution, bestFitness.ToString());
        }



        private void updateGraphThreadSafe(LiveCharts.WinForms.CartesianChart graph, double param)
        {
            if (cartesianChart1.InvokeRequired)
            {
                var d = new SafeGraphUpdateDelegate(updateGraphThreadSafe);
                graph.Invoke(d, new object[] { graph, param });
            }
            else
            {
                // the library can't manage values greater than float max, so cut them if necessary
                if (param > float.MaxValue)
                    param = float.MaxValue;
                graph.Series[0].Values.Add(param);
            }
        }



        private void updateProgress(double progress)
        {
            updateProgressBar(progress);
            updateRemainingTime(progress);
        }



        private void initProgressBar()
        {
            progressBar.Minimum = 0;
            progressBar.Maximum = PROGRESS_BAR_MAXIMUM;
            progressBar.Value = 0;
            progressBar.Visible = true;
        }



        private void updateProgressBar(double progress)
        {
            if (progressBar.InvokeRequired)
            {
                var d = new SafeProgressBarUpdate(updateProgressBar);
                progressBar.Invoke(d, new object[] { progress });
            }
            else
                progressBar.Value = (int)(progress * PROGRESS_BAR_MAXIMUM);
        }



        private void threadSafeEnableButton(Button button, bool enabled)
        {
            if (button.InvokeRequired)
            {
                var d = new SafeButtonEnabled(threadSafeEnableButton);
                button.Invoke(d, new object[] { button, enabled });
            }
            else
            {
                button.Enabled = enabled;
            }
        }



        private void updateRemainingTime(double progress)
        {
            if (progress >= MIN_PROGRESS_FOR_REM_TIME)
            {
                long millisSinceStart = OptimizationTimer.ElapsedMilliseconds;
                long totalMillis = (long)((double)millisSinceStart / progress);
                long remainingMillis = totalMillis - millisSinceStart;
                if (remainingMillis < 0)
                    remainingMillis = 0;

                int remainingSeconds = (int)remainingMillis / 1000;
                int remainingMinutes = remainingSeconds / 60;
                int remainingHours = remainingMinutes / 60;

                // format: x hours y minutes z seconds
                String remainingTimeStr = "";
                remainingTimeStr += remainingHours;
                remainingTimeStr += " " + Properties.Resources.hours + " ";
                remainingTimeStr += remainingMinutes % 60;
                remainingTimeStr += " " + Properties.Resources.minutes + " ";
                remainingTimeStr += remainingSeconds % 60;
                remainingTimeStr += " " + Properties.Resources.seconds;

                ThreadSafeLabelUpdate(labelRemainingTime, remainingTimeStr);
            }
        }



        void ThreadSafeLabelUpdate(Label label, String newValue)
        {
            if (label.InvokeRequired)
            {
                var d = new SafeLabelUpdate(ThreadSafeLabelUpdate);
                label.Invoke(d, new object[] { label, newValue });
            }
            else
            {
                label.Text = newValue;
            }
        }



        private void changeGraphParameter(LiveCharts.WinForms.CartesianChart graph, 
            out String paramVariable, String newParam)
        {
            try
            {
                MutGraph.WaitOne();
                paramVariable = newParam;
                graph.Series = new SeriesCollection
                {
                    new LineSeries
                    {
                        Title = newParam,
                        Values = new ChartValues<Double>(OptiParams[newParam]),
                        LineSmoothness = 0
                    }
                };

                graph.Refresh();
                MutGraph.ReleaseMutex();
            }
            catch (Exception e)
            {
                paramVariable = "";
                MessageBox.Show(Properties.Resources.UnknownError + "\n" + e.Message);
            }
        }



        // event handlers /////////////////////////////////////////////////////////////////////////



        private void comboBoxParam1_SelectedIndexChanged(object sender, EventArgs e)
        {
            String value = (String) comboBoxParam1.SelectedItem;
            changeGraphParameter(cartesianChart1, out GraphParam, value);
        }



        private void buttonSave_Click(object sender, EventArgs e)
        {
            saveFileDialogSaveCurrent.ShowDialog();
            String path = saveFileDialogSaveCurrent.FileName;
            if (!String.IsNullOrEmpty(path))
            {
                if (Optimizer.SaveCurrResult(path))
                    MessageBox.Show(Properties.Resources.SaveCurrentResultSuccess);
                else
                    MessageBox.Show(Properties.Resources.CantSaveCurrResult);

                saveFileDialogSaveCurrent.FileName = null;
            }
        }



        private async void buttonStop_Click(object sender, EventArgs e)
        {
            buttonStop.Enabled = false;
            // start the waiting animation
            progressBarStop.Visible = true;
            progressBarStop.Style = ProgressBarStyle.Marquee;
            labelStopping.Visible = true;
            bool stopRes = false;
            await Task.Run(() => (stopRes = Optimizer.Stop()));

            // hide the waiting animation
            progressBarStop.Visible = false;
            labelStopping.Visible = false;

            if (stopRes)
            {
                finish();
            }
            else
            {
                MessageBox.Show(Properties.Resources.CantStop);
            }
        }
    }
}
