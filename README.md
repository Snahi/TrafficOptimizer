# TrafficOptimizer
*Copyright © 2020 Szymon Niemczyk*  
Author: **Szymon Niemczyk**  
This is a software for traffic lights optimization. Optimization means here assigning such durations of traffic lights states (green/red) and offsets that the traffic flow is affected as little as possible. The optimization process is completely automatic once a user provides a model and optimization parameters. The methods used are based on the concept of differential evolution. The basic one is L-SHADE and the second one is L-SHADEWGD (developed by me as a modification of L-SHADE). I developed this program as a basis on which I wrote my bachelor thesis at Politechnika Wrocławska. This project was developed using Visual Studio and the Visual Studio files are also provided here so that you can easily build the project. 

# Architecture
In this section the basic structure of the project is described. Basically it is composed of two components: *TrafficOptimizerDLL* and *TrafficOptimizerUI*. The first one is the backend of the program and it is written in C++ as a *Dynamic Link Library*. The second component is the frontend part and it is written in C# using the Windows Forms framework. 
Several design patterns were used in order to make the software extensible and clean. Some of them are: 
* Observer (used in many cases, e.g. when a car finishes its route)
* Command (during optimization configuration)
* Facade (implemented models consist of many classes and in order to make the access easy and safe the facade design pattern was used in both of the models)
* Template Method (used in several places, e.g *iterate()* method in *Algorithm* or *evaluate()* in *Evaluator*
## TrafficOptimizerDLL
This component is the core of the entire software. It is written in C++ and it contains two simulations and the optimizer. The implemented simulations address different requirements. The first one (directory 'model') focuses on precision of representing the real world and allows for simulating probably any kind of a common system of junctions. The second one (directory 'simplified_model') focuses on efficiency and was made mainly for scientific purposes. The class responsible for optimization is *Optimizer*. To make it flexible some of its responsibilities were moved to other classes: *Algorithm* (exact method of optimization), *Evaluator* (evaluates quality of solutions), *Recorder* (stores result and / or statistics of the performed optimization in a file) and *StopCondition* (for deciding whether optimisation should be finished). 
In order to make the optimization accessible for the frontend component which is written in another programming language, *TrafficOptimizerDLL* is supposed to be compiled as a *Dynamic Link Library*. The accessible interface is defined in *TrafficOptimizer.h*. The *Optimization* class is responsible for managing the optimization, including providing thread safe access to *Optimizer*, e.g. starting a new optimization, getting access to the current state etc.
## TrafficOptimizerUI
This component is responsible for the user interface of this program. It is written in C#. It allows users to display results of past optimizations and run new ones. When the optimization is in progress it displays the current progress, estimated time till the end and a graph with the selected state parameter, e.g. the best fitness (quality of a solution in evolutionary algorithms nomenclature).

# How to use it?
In order to use this software you must use the Windows operating system. You can find the executable (TrafficOptimizerUI.exe) in the 'executable' directory. Then in order to run the optimization you will need to specify the optimization parameters and select the optimized model. For testing purposes you can use one of the provided models in the 'model_examples' directory. At this moment there is no tutorial for creating your own model files. Currently the user interface is available only in Polish.

# License
Copyright © 2020 Szymon Niemczyk
You cannot use this program with out the author's permission.
