/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/

#include "feat.h"

//shogun initialization
void __attribute__ ((constructor)) ctor()
{
    init_shogun_with_defaults();
}

void __attribute__ ((destructor))  dtor()
{
    exit_shogun();
    FT::Rnd::destroy();
    FT::Logger::destroy();
}

using namespace FT;
    
Feat::Feat(int pop_size, int gens, string ml, 
       bool classification, int verbosity, int max_stall,
       string sel, string surv, float cross_rate, float root_xo_rate,
       char otype, string functions, 
       unsigned int max_depth, unsigned int max_dim, int random_state, 
       bool erc, string obj, bool shuffle, 
       float split, float fb, string scorer, string feature_names,
       bool backprop,int iters, float lr, int bs, int n_threads,
       bool hillclimb, string logfile, int max_time, bool use_batch, bool residual_xo,
       bool stagewise_xo, bool softmax_norm, int print_pop, bool normalize):
          // construct subclasses
          params(pop_size, gens, ml, classification, max_stall, otype, verbosity, 
                 functions, cross_rate, root_xo_rate, max_depth, max_dim, erc, obj, shuffle, split, 
                 fb, scorer, feature_names, backprop, iters, lr, bs, hillclimb, max_time, 
                 use_batch, residual_xo, stagewise_xo, softmax_norm, normalize), 
          p_sel( make_shared<Selection>(sel) ),
          p_surv( make_shared<Selection>(surv, true) ),
          p_variation( make_shared<Variation>(cross_rate) ),
          print_pop(print_pop)
{
    r.set_seed(random_state);
    str_dim = "";
    set_logfile(logfile);
    this->scorer=scorer;
    if (n_threads!=0)
        omp_set_num_threads(n_threads);
    survival = surv;
    
    if (GPU)
        initialize_cuda();
    // set Feat's Normalizer to only normalize floats by default
    N.scale_all = false;
}

/// set size of population 
void Feat::set_pop_size(int pop_size){ params.pop_size = pop_size; }            

/// set size of max generations              
void Feat::set_generations(int gens){ params.gens = gens;}         
            
/// set ML algorithm to use              
void Feat::set_ml(string ml){ params.ml = ml; }            

/// set EProblemType for shogun              
void Feat::set_classification(bool classification){ params.classification = classification;}
     
/// set level of debug info              
void Feat::set_verbosity(int verbosity){ params.set_verbosity(verbosity); }
            
/// set maximum stall in learning, in generations
void Feat::set_max_stall(int max_stall){	params.max_stall = max_stall; }
            
/// set selection method              
void Feat::set_selection(string sel){ p_sel = make_shared<Selection>(sel); }
            
/// set survivability              
void Feat::set_survival(string surv)
{ 
    survival=surv; 
    p_surv = make_shared<Selection>(surv, true);
}
            
/// set cross rate in variation              
void Feat::set_cross_rate(float cross_rate)
{
    params.cross_rate = cross_rate; p_variation->set_cross_rate(cross_rate);
}
            
/// set root cross rate in variation              
void Feat::set_root_xo_rate(float cross_rate)
{
    params.root_xo_rate = cross_rate; 
}

/// set program output type ('f', 'b')              
void Feat::set_otype(char ot){ params.set_otype(ot); }
            
/// sets available functions based on comma-separated list.
void Feat::set_functions(string functions){ params.set_functions(functions); }
            
/// set max depth of programs              
void Feat::set_max_depth(unsigned int max_depth){ params.set_max_depth(max_depth); }
 
/// set maximum dimensionality of programs              
void Feat::set_max_dim(unsigned int max_dim){	params.set_max_dim(max_dim); }

///set dimensionality as multiple of the number of columns
void Feat::set_max_dim(string str) { str_dim = str; }            

/// set seeds for each core's random number generator              
void Feat::set_random_state(int random_state){ r.set_seed(random_state); }
            
/// flag to set whether to use variable or constants for terminals              
void Feat::set_erc(bool erc){ params.erc = erc; }

/// flag to shuffle the input samples for train/test splits
void Feat::set_shuffle(bool sh){params.shuffle = sh;}

/// set objectives in feat
void Feat::set_objectives(string obj){ params.set_objectives(obj); }

/// set train fraction of dataset
void Feat::set_split(float sp){params.split = sp;}

///set data types for input parameters
void Feat::set_dtypes(vector<char> dtypes){params.dtypes = dtypes;}

///set feedback
void Feat::set_feedback(float fb){ params.feedback = fb;}

///set name for files
void Feat::set_logfile(string s){logfile = s;}

///set scoring function
void Feat::set_scorer(string s){scorer=s; params.scorer=s;}

void Feat::set_feature_names(string s){params.set_feature_names(s);}
void Feat::set_feature_names(vector<string>& s){params.feature_names = s;}

/// set constant optimization options
void Feat::set_backprop(bool bp){params.backprop=bp;}

void Feat::set_hillclimb(bool hc){params.hillclimb=hc;}

void Feat::set_iters(int iters){params.bp.iters = iters; params.hc.iters=iters;}

void Feat::set_lr(float lr){params.bp.learning_rate = lr;}

void Feat::set_batch_size(int bs){params.bp.batch_size = bs;}
 
///set number of threads
void Feat::set_n_threads(unsigned t){ omp_set_num_threads(t); }

void Feat::set_max_time(int time){ params.max_time = time; }

void Feat::set_use_batch(){ params.use_batch = true; }

/*                                                      
 * getting functions
 */

///return population size
int Feat::get_pop_size(){ return params.pop_size; }

///return size of max generations
int Feat::get_generations(){ return params.gens; }

///return ML algorithm string
string Feat::get_ml(){ return params.ml; }

///return type of classification flag set
bool Feat::get_classification(){ return params.classification; }

///return maximum stall in learning, in generations
int Feat::get_max_stall() { return params.max_stall; }

///return program output type ('f', 'b')             
vector<char> Feat::get_otypes(){ return params.otypes; }

///return current verbosity level set
int Feat::get_verbosity(){ return params.verbosity; }

///return max_depth of programs
int Feat::get_max_depth(){ return params.max_depth; }

///return cross rate for variation
float Feat::get_cross_rate(){ return params.cross_rate; }

///return max size of programs
int Feat::get_max_size(){ return params.max_size; }

///return max dimensionality of programs
int Feat::get_max_dim(){ return params.max_dim; }

///return boolean value of erc flag
bool Feat::get_erc(){ return params.erc; }

/// get name
string Feat::get_logfile(){ return logfile; }

///return number of features
int Feat::get_num_features(){ return params.num_features; }

///return whether option to shuffle the data is set or not
bool Feat::get_shuffle(){ return params.shuffle; }

///return fraction of data to use for training
float Feat::get_split(){ return params.split; }

///add custom node into feat
/* void add_function(unique_ptr<Node> N){ params.functions.push_back(N->clone()); } */

///return data types for input parameters
vector<char> Feat::get_dtypes(){ return params.dtypes; }

///return feedback setting
float Feat::get_feedback(){ return params.feedback; }

///return best model
string Feat::get_representation(){ return best_ind.get_eqn();}

string Feat::get_model()
{   
    vector<string> features = best_ind.get_features();
    vector<float> weights = best_ind.ml->get_weights();
    
    vector<float> aweights(weights.size());
    for (int i =0; i<aweights.size(); ++i) 
        aweights[i] = fabs(weights[i]);
    vector<size_t> order = argsort(aweights);
    string output;
    output += "Feature\t\tWeight\n";
    for (unsigned i = order.size(); i --> 0;)
    {
        output += features.at(order[i]);
        output += "\t\t";
        output += std::to_string(weights.at(order[i]));
        output += "\n";
    }
    return output;
}

///get number of parameters in best
int Feat::get_n_params(){ return best_ind.get_n_params(); } 

///get dimensionality of best
int Feat::get_dim(){ return best_ind.get_dim(); } 

///get dimensionality of best
int Feat::get_complexity(){ return best_ind.complexity(); } 


/// return the number of nodes in the best model
int Feat::get_n_nodes(){ return best_ind.program.size(); }

///return population as string
string Feat::get_eqns(bool front)
{
    string r="complexity\tfitness\tfitness_v\teqn\n";
    if (front)  // only return individuals on the Pareto front
    {
        if (use_arch)
        {
            // printing individuals from the archive 
            unsigned n = 1;
            
            for (auto& a : arch.archive)
            {          
                r += std::to_string(a.complexity()) + "\t" 
                    + std::to_string(a.fitness) + "\t" 
                    + std::to_string(a.fitness_v) + "\t"
                    + a.get_eqn() + "\n";  
            }
        }
        else
        {
            // printing individuals from the pareto front
            unsigned n = 1;
            vector<size_t> f = p_pop->sorted_front(n);
            
            for (unsigned j = 0; j < f.size(); ++j)
            {          
                r += std::to_string(p_pop->individuals[f[j]].complexity()) + "\t" 
                    + std::to_string((*p_pop)[f[j]].fitness) + "\t" 
                    + std::to_string((*p_pop)[f[j]].fitness_v) + "\t" 
                    + p_pop->individuals[f[j]].get_eqn() + "\n";  
            }
        }
    }
    else
    {
        for (unsigned j = 0; j < params.pop_size; ++j)
        {          
            r += std::to_string(p_pop->individuals[j].complexity()) + "\t" 
                + std::to_string((*p_pop)[j].fitness) + "\t" 
                + std::to_string((*p_pop)[j].fitness_v) + "\t" 
                + p_pop->individuals[j].get_eqn() + "\n";  
        }
    }
    return r;
}

/// return the coefficients or importance scores of the best model. 
ArrayXf Feat::get_coefs()
{
    auto tmpw = best_ind.ml->get_weights();
    ArrayXf w = ArrayXf::Map(tmpw.data(), tmpw.size());
    return w;
}

/// get longitudinal data from file s
std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf>>> Feat::get_Z(string s, 
        int * idx, int idx_size)
{
    std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z;
    vector<int> ids(idx,idx+idx_size);
    load_partial_longitudinal(s,Z,',',ids);
        
    return Z;
}

/// destructor             
Feat::~Feat(){} 
            
ArrayXXf Feat::predict_proba(float * X, int rows_x, int cols_x) 
{			    
    MatrixXf matX = Map<MatrixXf>(X,rows_x,cols_x);
    return predict_proba(matX);
}

/// convenience function calls fit then predict.            
VectorXf Feat::fit_predict(MatrixXf& X,
                     VectorXf& y,
                     std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
                     { fit(X, y, Z); return predict(X, Z); } 
                     
VectorXf Feat::fit_predict(float * X, int rows_x, int cols_x, float * Y, int len_y)
{
    MatrixXf matX = Map<MatrixXf>(X,rows_x,cols_x);
    VectorXf vectY = Map<VectorXf>(Y,len_y);
    fit(matX,vectY); 
    return predict(matX); 
} 

/// convenience function calls fit then transform. 
MatrixXf Feat::fit_transform(MatrixXf& X,
                       VectorXf& y,
                       std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
                       { fit(X, y, Z); return transform(X, Z); }                                         

void Feat::fit(MatrixXf& X, VectorXf& y,
               std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
{

    /*! 
     *  Input:
     
     *       X: n_features x n_samples MatrixXf of features
     *       y: VectorXf of labels 
     
     *  Output:
     
     *       updates best_estimator, hof
    
     *   steps:
     *	   1. fit model yhat = f(X)
     *	   2. generate transformations Phi(X) for each individual
     *	   3. fit model yhat_new = f( Phi(X)) for each individual
     *	   4. evaluate features
     *	   5. selection parents
     *	   6. produce offspring from parents via variation
     *	   7. select surviving individuals from parents and offspring
     */

    // start the clock
    timer.Reset();

    if (params.use_batch)
    {
        if (params.bp.batch_size >= X.cols()){
            cout << "turning off batch because X has fewer than " << params.bp.batch_size 
                 << " samples\n";
            params.use_batch = false;
        }
        else
            cout << "using batch with batch_size= " << params.bp.batch_size << "\n";
    }
    std::ofstream log;                      ///< log file stream
    if (!logfile.empty())
        log.open(logfile, std::ofstream::app);
    
    if(str_dim.compare("") != 0)
    {
        string dimension;
        dimension = str_dim.substr(0, str_dim.length() - 1);
        logger.log("STR DIM IS "+ dimension, 2);
        logger.log("Cols are " + std::to_string(X.rows()), 2);
        logger.log("Setting dimensionality as " + 
                   std::to_string((int)(ceil(stod(dimension)*X.rows()))), 2);
        set_max_dim(ceil(stod(dimension)*X.rows()));
    }
    
    params.init();       
  

    if (params.classification)  // setup classification endpoint
    {
       params.set_classes(y);       
       params.set_scorer(scorer);
    } 
    
    if (params.dtypes.size()==0)    // set feature types if not set
        set_dtypes(find_dtypes(X));
   
    if (params.normalize)
        N.fit_normalize(X,params.dtypes);                   // normalize data
    /* p_ml = make_shared<ML>(params); // intialize ML */
    p_pop = make_shared<Population>(params.pop_size);
    p_eval = make_shared<Evaluation>(params.scorer);

    // create an archive to save Pareto front, unless NSGA-2 is being used for survival 
    if (!survival.compare("nsga2"))
        use_arch = false;
    else
        use_arch = true;
    
    // split data into training and test sets
    //Data data(X, y, Z, params.classification);
    DataRef d(X, y, Z, params.classification);
    //DataRef d;
    //d.setOriginalData(&data);
    d.train_test_split(params.shuffle, params.split);
    // define terminals based on size of X
    params.set_terminals(d.o->X.rows(), d.o->Z);        

    // initial model on raw input
    logger.log("Setting up data", 2);
    float t0 =  timer.Elapsed().count();
    
    //data for batch training
    MatrixXf Xb;
    VectorXf yb;
    std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Zb;
    Data db(Xb, yb, Zb, params.classification);
    
    Data *tmp_train;
    
    if(params.use_batch)
    {
        tmp_train = d.t;
        d.t->get_batch(db, params.bp.batch_size);
        d.setTrainingData(&db);
    }
    
    if (params.classification) 
        params.set_sample_weights(d.t->y); 
    
    // initial model
    logger.log("Fitting initial model", 2);
    t0 =  timer.Elapsed().count();
    initial_model(d);  
    logger.log(std::to_string(timer.Elapsed().count() - t0) + " seconds",2);
    // initialize population 
    logger.log("Initializing population", 2);
   
    bool random = false;
    if (!p_sel->get_type().compare("random"))
        random = true;

    p_pop->init(best_ind,params,random);
    logger.log("Initial population:\n"+p_pop->print_eqns(),3);

    // resize F to be twice the pop-size x number of samples
    F.resize(d.t->X.cols(),int(2*params.pop_size));
   
    // evaluate initial population
    logger.log("Evaluating initial population",2);
    p_eval->fitness(p_pop->individuals,*d.t,F,params);
    
    logger.log("Initial population done",2);
    logger.log(std::to_string(timer.Elapsed().count()) + " seconds",2);
    
    vector<size_t> survivors;
    
    if(params.use_batch)    // reset d to all training data
        d.setTrainingData(tmp_train, true);

    // =====================
    // main generational loop
    unsigned g = 0;
    unsigned stall_count = 0;
    float fraction = 0;
    // continue until max gens is reached or max_time is up (if it is set)
    
    while((params.max_time == -1 || params.max_time > timer.Elapsed().count()) // time limit
           && g<params.gens                                                    // generation limit
           && (params.max_stall == 0 || stall_count < params.max_stall) )      // stall limit
    {
        fraction = params.max_time == -1 ? ((g+1)*1.0)/params.gens : 
                                           timer.Elapsed().count()/params.max_time;
        if(params.use_batch)
        {
            d.t->get_batch(db, params.bp.batch_size);
            DataRef dbr;    // reference to minibatch data
            dbr.setTrainingData(&db);
            dbr.setValidationData(d.v);

            if (params.classification)
                params.set_sample_weights(dbr.t->y); 

            run_generation(g, survivors, dbr, log, fraction, stall_count);
        }
        else
            run_generation(g, survivors, d, log, fraction, stall_count);
        
        g++;
    }
    // =====================
    if ( params.max_stall != 0 && stall_count >= params.max_stall)
        logger.log("learning stalled",2);
    else if ( g >= params.gens) 
        logger.log("generation limit reached",2);
    else
        logger.log("max time reached",2);
    logger.log("best training representation: " + best_ind.get_eqn(),2);
    logger.log("train score: " + std::to_string(best_score), 2);
    // evaluate population on validation set
    if (params.split < 1.0)
    {
        vector<Individual>& final_pop = use_arch ? arch.archive : p_pop->individuals; 
        
        F_v.resize(d.v->X.cols(),int(2*params.pop_size)); 
        
        p_eval->fitness(final_pop, *d.v, F_v, params, false, true);
        // print validation scores
        if (params.verbosity > 2)
        {
            string scores = "";
            for (auto& ind : final_pop)
                scores += ind.get_eqn() + "; score: " + std::to_string(ind.fitness_v) + "\n";
            logger.log(scores,3);
        }
        update_best(d,true);                  // get the best validation model
    }
    else
        best_score_v = best_score;
   
    logger.log("best validation representation: " + best_ind.get_eqn(),2);
    logger.log("validation score: " + std::to_string(best_score_v), 2);
    logger.log("fitting final model to all training data...",2);

    final_model(d);   // fit final model to best features
    logger.log("\nTotal time taken is " + std::to_string(timer.Elapsed().count()) + "\n", 1);
    
    if (log.is_open())
        log.close();
}

void Feat::run_generation(unsigned int g,
                      vector<size_t> survivors,
                      DataRef &d,
                      std::ofstream &log,
                      float fraction,
                      unsigned& stall_count)
{
    params.set_current_gen(g);

    // select parents
    logger.log("selection..", 3);
    vector<size_t> parents = p_sel->select(*p_pop, F, params);
    logger.log("parents:\n"+p_pop->print_eqns(), 3);          
    
    // variation to produce offspring
    logger.log("variation...", 3);
    p_variation->vary(*p_pop, parents, params,*d.t);
    logger.log("offspring:\n" + p_pop->print_eqns(true), 3);

    // evaluate offspring
    logger.log("evaluating offspring...", 3);
    p_eval->fitness(p_pop->individuals, *d.t, F, params, true && !params.use_batch);
    // select survivors from combined pool of parents and offspring
    logger.log("survival...", 3);
    survivors = p_surv->survive(*p_pop, F, params);
   
    // reduce population to survivors
    logger.log("shrinking pop to survivors...",3);
    p_pop->update(survivors);
    logger.log("survivors:\n" + p_pop->print_eqns(), 3);
    
    update_best(d);

    if (params.max_stall > 0)
        update_stall_count(stall_count, F, d);

    if (use_arch) 
        arch.update(*p_pop,params);

    calculate_stats();
    
    if(params.verbosity>1)
        print_stats(log, fraction);    
    else if(params.verbosity == 1)
        printProgress(fraction);
    
    if (print_pop > 1 || print_pop > 0 && g == params.gens-1)
        print_population();

    if (params.backprop)
    {
        params.bp.learning_rate = (1-1/(1+float(params.gens)))*params.bp.learning_rate;
        logger.log("learning rate: " + std::to_string(params.bp.learning_rate),3);
    }

}

void Feat::update_stall_count(unsigned& stall_count, MatrixXf& F, const DataRef& d)
{
    /* double med_score = median(F.colwise().mean().array());  // median loss */
    vector<float> fitnesses;
    for (unsigned i = 0; i < p_pop->individuals.size(); ++i)
        fitnesses.push_back(p_pop->individuals.at(i).fitness);
    int idx = argmiddle(fitnesses);
    /* cout << "fitnesses: \n"; */
    /* for (const auto& f : fitnesses) cout << f << ", "; cout << "\n"; */
    /* cout << "idx: " << idx << "\n"; */
    Individual& med_ind = p_pop->individuals.at(idx);
    /* cout << "med_ind: " << med_ind.get_eqn() << "\n"; */
    VectorXf tmp;
    shared_ptr<CLabels> yhat_v = med_ind.predict(*d.v, params);
    med_loss_v = p_eval->score(d.v->y, yhat_v, tmp, params.class_weights); 

    if (params.current_gen == 0 || med_loss_v < best_med_score)
    {
        /* cout << "updating best_med_score to " << med_loss_v << "\n"; */
        best_med_score = med_loss_v;
        stall_count = 0;
    }
    else
    {
        ++stall_count;
    }

    logger.log("stall count: " + std::to_string(stall_count), 2);
}
void Feat::fit(float * X, int rowsX, int colsX, float * Y, int lenY)
{
    MatrixXf matX = Map<MatrixXf>(X,rowsX,colsX);
    VectorXf vectY = Map<VectorXf>(Y,lenY);

    Feat::fit(matX,vectY);
}

void Feat::fit_with_z(float * X, int rowsX, int colsX, float * Y, int lenY, string s, 
                int * idx, int idx_size)
{

    MatrixXf matX = Map<MatrixXf>(X,rowsX,colsX);
    VectorXf vectY = Map<VectorXf>(Y,lenY);
    auto Z = get_Z(s, idx, idx_size);
    // TODO: make sure long fns are set
    /* string longfns = "mean,median,max,min,variance,skew,kurtosis,slope,count"; */

    fit(matX,vectY,Z); 
}

void Feat::final_model(DataRef& d)	
{
    // fits final model to best tranformation found.
    bool pass = true;

    /* MatrixXf Phi = transform(X); */
    /* MatrixXf Phi = best_ind.out(*d.o, params); */        
    
    shared_ptr<CLabels> yhat = best_ind.fit(*d.o, params, pass);
    VectorXf tmp;
    /* params.set_sample_weights(y);   // need to set new sample weights for y, */ 
                                    // which is probably from a validation set
    float score = p_eval->score(d.o->y,yhat,tmp,params.class_weights);
    logger.log("final_model score: " + std::to_string(score),2);
}

void Feat::initial_model(DataRef &d)
{
    /*!
     * fits an ML model to the raw data as a starting point.
     */
     
    best_ind = Individual();
    best_ind.set_id(0);
    int j; 
    int n_feats = std::min(params.max_dim, unsigned(d.t->X.rows()));
    
    vector<size_t> var_idx(d.t->X.rows());
    std::iota(var_idx.begin(),var_idx.end(),0);
    
    for (unsigned i =0; i<n_feats; ++i)
    {
        if (n_feats < d.t->X.rows())
        {
            vector<size_t> choice_idxs(d.t->X.rows()-i);
            std::iota(choice_idxs.begin(),choice_idxs.end(),0);
            /* cout << "choice_idxs: "; */
            /* for (auto c : choice_idxs) cout << c << ","; cout << "\n"; */
            size_t idx = r.random_choice(choice_idxs);
            /* cout << "idx: " << idx << "\n"; */
            j = var_idx.at(idx);
            /* cout << "j: " << j << "\n"; */
            var_idx.erase(var_idx.begin() + idx);
        }
        else 
            j=i;
        best_ind.program.push_back(params.terminals.at(j)->clone());
    }
    // if there is longitudinal data, initialize the model with median values applied to those
    // variables.
    int n_long_feats = std::min(params.max_dim - n_feats, unsigned(d.t->Z.size()));
    int n_x = d.t->X.rows();
    cout << "n_long_feats: " << n_long_feats << "\n";
    vector<size_t> zvar_idx(d.t->Z.size());
    std::iota(zvar_idx.begin(),zvar_idx.end(),0);
    
    for (unsigned i =0; i<n_long_feats; ++i)
    {
        if (n_long_feats < d.t->Z.size())
        {
            vector<size_t> choice_idxs(d.t->Z.size()-i);
            std::iota(choice_idxs.begin(),choice_idxs.end(),0);
            /* cout << "choice_idxs: "; */
            /* for (auto c : choice_idxs) cout << c << ","; cout << "\n"; */
            size_t idx = r.random_choice(choice_idxs);
            /* cout << "idx: " << idx << "\n"; */
            j = zvar_idx.at(idx);
            /* cout << "j: " << j << "\n"; */
            zvar_idx.erase(zvar_idx.begin() + idx);
        }
        else 
            j=i;
        best_ind.program.push_back(params.terminals.at(n_x + j)->clone());
        best_ind.program.push_back(std::unique_ptr<Node>(new NodeMedian()));
    }
    /* best_ind.program.push_back(std::unique_ptr<Node>(new NodeLongitudinal("bmi"))); */
    /* best_ind.program.push_back(std::unique_ptr<Node>(new NodeSlope())); */
    //
    cout << "initial model: " << best_ind.get_eqn() << "\n";
    //
    bool pass = true;
    shared_ptr<CLabels> yhat = best_ind.fit(*d.t,params,pass);
    /* SGVector<double> _Tmp = dynamic_pointer_cast<sh::CBinaryLabels>(yhat)->get_labels(); */
    /* SGVector<float> Tmp(_Tmp.begin(), _Tmp.end()); */
    /* Map<VectorXf> yhatV(Tmp.data(),Tmp.size()); */
    /* cout << "yhat: " << yhatV.transpose() << "\n"; */
    /* cout << "y: " << d.t->y.transpose() << "\n"; */
    /* ArrayXf diff = yhatV.array() - d.t->y.array() ; */ 
    /* cout << "diff: " << diff.transpose() << "\n"; */ 
    // set terminal weights based on model
    vector<float> w;
    if (n_feats + n_long_feats == d.t->X.rows() + d.t->Z.size())
        w = best_ind.ml->get_weights();
    /* else */
    /*     w = vector<float>(d.t->X.rows()+d.t->Z.size(), */
    /*                       1.0/float(params.terminals.size())); */
        
    
    params.set_term_weights(w);
   
    VectorXf tmp;
    best_score = p_eval->score(d.t->y, yhat, tmp, params.class_weights);
    

    if (params.split < 1.0)
    {
        shared_ptr<CLabels> yhat_v = best_ind.predict(*d.v, params);
        best_score_v = p_eval->score(d.v->y, yhat_v, tmp, params.class_weights); 
    }
    else
        best_score_v = best_score;
    
    best_ind.fitness = best_score;
    
    logger.log("initial training score: " +std::to_string(best_score),2);
    logger.log("initial validation score: " +std::to_string(best_score_v),2);
}

MatrixXf Feat::transform(MatrixXf& X,
                         std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z,
                         Individual *ind)
{
    /*!
     * Transforms input data according to ind or best ind, if ind is undefined.
     */
    
    if (params.normalize)
        N.normalize(X);       
    
    VectorXf y = VectorXf();
    
    Data d(X, y, Z, get_classification());
    
    if (ind == 0)        // if ind is empty, predict with best_ind
    {
        if (best_ind.program.size()==0)
            HANDLE_ERROR_THROW("You need to train a model using fit() before making predictions.");
        
        return best_ind.out(d, params, true);
    }

    return ind->out(d, params, true);
}

MatrixXf Feat::transform(float * X, int rows_x, int cols_x)
{
    MatrixXf matX = Map<MatrixXf>(X,rows_x,cols_x);
    return transform(matX);
    
}

MatrixXf Feat::transform_with_z(float * X, int rowsX, int colsX, string s, int * idx, int idx_size)
{
    MatrixXf matX = Map<MatrixXf>(X,rowsX,colsX);
    auto Z = get_Z(s, idx, idx_size);
    
    return transform(matX, Z);
    
}

MatrixXf Feat::fit_transform(float * X, int rows_x, int cols_x, float * Y, int len_y)
{
    MatrixXf matX = Map<MatrixXf>(X,rows_x,cols_x);
    VectorXf vectY = Map<VectorXf>(Y,len_y);
    fit(matX,vectY); 
    return transform(matX);
}

VectorXf Feat::predict(MatrixXf& X,
                       std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
{        
    MatrixXf Phi = transform(X, Z);
    return best_ind.ml->predict_vector(Phi);        
}

shared_ptr<CLabels> Feat::predict_labels(MatrixXf& X,
                       std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
{        
    MatrixXf Phi = transform(X, Z);
    return best_ind.ml->predict(Phi);        
}

VectorXf Feat::predict(float * X, int rowsX,int colsX)
{
    MatrixXf matX = Map<MatrixXf>(X,rowsX,colsX);
    return predict(matX);
}

VectorXf Feat::predict_with_z(float * X, int rowsX,int colsX, 
                                string s, int * idx, int idx_size)
{

    MatrixXf matX = Map<MatrixXf>(X,rowsX,colsX);
    auto Z = get_Z(s, idx, idx_size);
    // TODO: make sure long fns are set
    /* string longfns = "mean,median,max,min,variance,skew,kurtosis,slope,count"; */

    return predict(matX,Z); 
}

ArrayXXf Feat::predict_proba(MatrixXf& X,
                         std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
{
    MatrixXf Phi = transform(X, Z);
    return best_ind.ml->predict_proba(Phi);        
}

ArrayXXf Feat::predict_proba_with_z(float * X, int rowsX,int colsX, 
                                string s, int * idx, int idx_size)
{
    MatrixXf matX = Map<MatrixXf>(X,rowsX,colsX);
    auto Z = get_Z(s, idx, idx_size);
    // TODO: make sure long fns are set
    /* string longfns = "mean,median,max,min,variance,skew,kurtosis,slope,count"; */

    return predict_proba(matX,Z); 
}


void Feat::update_best(const DataRef& d, bool validation)
{
    logger.log("updating best..",2);

    float bs;
    bs = validation ? best_score_v : best_score ; 
    float f; 
    vector<Individual>& pop = use_arch && validation ? arch.archive : p_pop->individuals; 

    for (const auto i: pop)
    {
        f = validation ? i.fitness_v : i.fitness ;
        if (f < bs)
        {
            bs = f;
            best_ind = i; // should this be i.clone(best_ind); ?
            /* i.clone(best_ind); */
        }
    }

    if (validation) 
        best_score_v = bs; 
    else
    {
        best_score = bs;

        if (params.split < 1.0)
        {
            VectorXf tmp;
            shared_ptr<CLabels> yhat_v = best_ind.predict(*d.v, params);
            best_score_v = p_eval->score(d.v->y, yhat_v, tmp, params.class_weights); 
            best_ind.fitness_v = best_score_v;
        }
    }
}

float Feat::score(MatrixXf& X, const VectorXf& y,
                   std::map<string, std::pair<vector<ArrayXf>, vector<ArrayXf> > > Z)
{
    shared_ptr<CLabels> labels = predict_labels(X, Z);
    VectorXf loss; 
    return p_eval->score(y,labels,loss,params.class_weights);
}

void Feat::calculate_stats()
{

    float med_score = median(F.colwise().mean().array());  // median loss
    
    ArrayXf Sizes(p_pop->size());
    
    unsigned i = 0;
    
    for (const auto& p : p_pop->individuals){ Sizes(i) = p.size(); ++i;}
    unsigned med_size = median(Sizes);                        // median program size
    
    ArrayXf Complexities(p_pop->size()); 
    i = 0; for (auto& p : p_pop->individuals){ Complexities(i) = p.complexity(); ++i;}
    ArrayXf Nparams(p_pop->size()); 
    i = 0; for (auto& p : p_pop->individuals){ Nparams(i) = p.get_n_params(); ++i;}
    ArrayXf Dims(p_pop->size()); 
    i = 0; for (auto& p : p_pop->individuals){ Dims(i) = p.get_dim(); ++i;}
    
    /* unsigned med_size = median(Sizes);                        // median program size */
    unsigned med_complexity = median(Complexities);           // median 
    unsigned med_num_params = median(Nparams);                // median program size
    unsigned med_dim = median(Dims);                          // median program size
    
    
    stats.update(params.current_gen,
                 timer.Elapsed().count(),
                 best_score,
                 best_score_v,
                 med_score,
                 med_loss_v,
                 med_size,
                 med_complexity,
                 med_num_params,
                 med_dim);
}

void Feat::print_stats(std::ofstream& log, float fraction)
{
    unsigned num_models = std::min(50,p_pop->size());
    //float med_score = median(F.colwise().mean().array());  // median loss
    ArrayXf Sizes(p_pop->size()); unsigned i = 0;           // collect program sizes
    for (const auto& p : p_pop->individuals){ Sizes(i) = p.size(); ++i;}
    unsigned max_size = Sizes.maxCoeff();
    string bar, space = "";                                 // progress bar
    for (unsigned int i = 0; i<50; ++i)
    {
        if (i <= 50*fraction) bar += "/";
        else space += " ";
    }
    std::cout.precision(5);
    std::cout << std::scientific;
    
    if(params.max_time == -1)
        std::cout << "Generation " << params.current_gen+1 << "/" << params.gens 
                  << " [" + bar + space + "]\n";
    else
        std::cout << std::fixed << "Time elapsed "<< timer << "/" << params.max_time <<
                     " seconds (Generation "<< params.current_gen+1 <<") [" + bar + space + "]\n";
        
    std::cout << "Min Loss\tMedian Loss\tMedian (Max) Size\tTime (s)\n"
              <<  best_score << "\t" << stats.med_score.back() << "\t" ;
    std::cout << std::fixed  << stats.med_size.back() << " (" << max_size << ") \t\t" << timer << "\n";
    std::cout << "Representation Pareto Front--------------------------------------\n";
    std::cout << "Rank\tComplexity\tLoss\tRepresentation\n";
    std::cout << std::scientific;
    // printing 10 individuals from the pareto front
    unsigned n = 1;
    if (use_arch)
    {
        num_models = std::min(40, int(arch.archive.size()));

        for (unsigned i = 0; i < num_models; ++i)
        {
            std::string lim_model;
            std::string model = arch.archive[i].get_eqn();
            for (unsigned j = 0; j< std::min(model.size(),size_t(60)); ++j)
            {
                lim_model.push_back(model.at(j));
            }
            if (lim_model.size()==60) 
                lim_model += "...";
            
            std::cout <<  arch.archive[i].rank          << "\t" 
                      <<  arch.archive[i].complexity()  << "\t" 
                      <<  arch.archive[i].fitness       << "\t" 
                      <<  lim_model << "\n";  
        }
    }
    else
    {
        vector<size_t> f = p_pop->sorted_front(n);
        vector<size_t> fnew(2,0);
        while (f.size() < num_models && fnew.size()>1)
        {
            fnew = p_pop->sorted_front(++n);                
            f.insert(f.end(),fnew.begin(),fnew.end());
        }
        
        for (unsigned j = 0; j < std::min(num_models,unsigned(f.size())); ++j)
        {     
            std::string lim_model;
            std::string model = p_pop->individuals[f[j]].get_eqn();
            for (unsigned j = 0; j< std::min(model.size(),size_t(60)); ++j)
                lim_model.push_back(model.at(j));
            if (lim_model.size()==60) 
                lim_model += "...";
            std::cout << p_pop->individuals[f[j]].rank              << "\t" 
                      <<  p_pop->individuals[f[j]].complexity()     << "\t" 
                      << (*p_pop)[f[j]].fitness                     << "\t"
                      << "\t" << lim_model << "\n";  
        }
    }
   
    std::cout <<"\n\n";
    
    /* float med_score = median(F.colwise().mean().array());  // median loss */
    /* ArrayXf Sizes(p_pop->size());                           // collect program sizes */
    /* i = 0; for (auto& p : p_pop->individuals){ Sizes(i) = p.size(); ++i;} */ 

    if (!logfile.empty())
    {
        // print stats in tabular format
        string sep = ",";
        if (params.current_gen == 0) // print header
        {
            log << "generation"     << sep
                << "time"           << sep
                << "min_loss"       << sep 
                << "min_loss_val"   << sep 
                << "med_loss"       << sep 
                << "med_loss_val"   << sep 
                << "med_size"       << sep 
                << "med_complexity" << sep 
                << "med_num_params" << sep
                << "med_dim\n";
        }

        log << params.current_gen  << sep
            << timer.Elapsed().count() << sep
            << best_score          << sep
            << best_score_v        << sep
            << stats.med_score.back()  << sep
            << med_loss_v          << sep
            << stats.med_size.back()   << sep
            << stats.med_complexity.back() << sep
            << stats.med_num_params.back() << sep
            << stats.med_dim.back()        << "\n"; 
    } 
}
void Feat::print_population()
{
    std::ofstream out;                      ///< log file stream
    string sep = "\t";
    if (!logfile.empty())
        out.open(logfile + ".pop" + std::to_string(params.current_gen));
    else
        out.open("pop" + std::to_string(params.current_gen));

    out << "id" << sep << "parent_id" << sep; 
    for (const auto& o : params.objectives)
        out << o << sep;
    out << "rank" << sep << "eqn";
    out << "\n"; 
    /* out << "\n"; */
    for (auto& i : p_pop->individuals)
    {
        out << i.id << sep; 
        for (unsigned j = 0; j<i.parent_id.size(); ++j) 
        {
            if (j > 0) 
                out << ",";
            out << i.parent_id.at(j) ;
        }
        out << sep ;
        for (const auto& o : i.obj)
            out << o << sep;
        out << i.rank << sep;
        out << i.get_eqn() ;
        out << "\n";
    }
    out.close();
}

vector<int> Feat::get_gens(){return stats.generation;}

vector<float> Feat::get_timers(){return stats.time;}

vector<float> Feat::get_best_scores(){return stats.best_score;}

vector<float> Feat::get_best_score_vals(){return stats.best_score_v;}

vector<float> Feat::get_med_scores(){return stats.med_score;}

vector<float> Feat::get_med_loss_vals(){return stats.med_loss_v;}

vector<unsigned> Feat::get_med_size(){return stats.med_size;}

vector<unsigned> Feat::get_med_complexities(){return stats.med_complexity;}

vector<unsigned> Feat::get_med_num_params(){return stats.med_num_params;}

vector<unsigned> Feat::get_med_dim(){return stats.med_dim;}
