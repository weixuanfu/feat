/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#include "n_greaterthan.h"
    	   
namespace FT{
	
    namespace Pop{
        namespace Op{    
            NodeGreaterThan::NodeGreaterThan()
            {
	            name = ">";
	            otype = 'b';
	            arity['f'] = 2;
	            complexity = 2;
            }

            /// Evaluates the node and updates the state states. 
            void NodeGreaterThan::evaluate(const Data& data, State& state)
            {
                ArrayXd x1 = state.pop<double>();
                ArrayXd x2 = state.pop<double>();
                state.push<bool>(x1 > x2);
            }

            /// Evaluates the node symbolically
            void NodeGreaterThan::eval_eqn(State& state)
            {
                state.push<bool>("(" + state.popStr<double>() + ">" + state.popStr<double>() + ")");
            }
            
            NodeGreaterThan* NodeGreaterThan::clone_impl() const { return new NodeGreaterThan(*this); }
              
            NodeGreaterThan* NodeGreaterThan::rnd_clone_impl() const { return new NodeGreaterThan(); } 
        }
    }
}