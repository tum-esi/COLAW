## Evaluating scenarios

The data generated from the simulation are automatically stored in the **results** directory. For the matter of traceability and future
analysis of the data the directory is also submitted as part of our work. The generated data is sorted into four sub-directories that 
correspond to the analyzed scenarios. The name convention followed is *\<Scenario\>_\<DecisionLogic\>*. The data generated from
OMNeT++ is started in the *&ast;.sca*, *&ast;.vci* and *&ast;.vec* files. To generate the *&ast;.csv* file that is used to evaluate 
the data and generate the plots you have to run 

    ./analyze_traces <Scenario>/<Scenario>_<DecisionLogic>

e.g. **./analyze_traces.sh No_Obstacles/No_Obstacles_80**. 
This will generate the *&ast;.csv* file and two plots. The *&ast;_relationship* plot
lists the number of generated proofs depending on the size of the requested proof size, colorcoding the size of the proof. The 
*&ast;.size* adds together all the generated proofs corresponding to one size. Both plot are stored as a *&ast;.pdf* (for preview) and
as a *&ast;.pgf* for LaTeX inclusion.