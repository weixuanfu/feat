/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef NODE_EXPONENTIAL
#define NODE_EXPONENTIAL

#include "../n_Dx.h"

namespace FT{

    namespace Pop{
        namespace Op{
        	class NodeExponential : public NodeDx
            {
            	public:
           	
            		NodeExponential(vector<double> W0 = vector<double>());
            		
                    /// Evaluates the node and updates the stack states. 
                    void evaluate(const Data& data, Stacks& stack);

                    /// Evaluates the node symbolically
                    void eval_eqn(Stacks& stack);

                    ArrayXd getDerivative(Trace& stack, int loc);

                protected:
                    NodeExponential* clone_impl() const override;
              
                    NodeExponential* rnd_clone_impl() const override;
            };
        }
    }
}	

#endif