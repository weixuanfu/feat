/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/

#include "n_variable.h"
			
namespace FT{
    template <class T>
    NodeVariable<T>::NodeVariable(const size_t& l, char ntype, std::string n)
    {
        if (n.empty())
	        name = "x_" + std::to_string(l);
        else
            name = n;
	    otype = ntype;
	    complexity = 1;
	    loc = l;
    }

    /// Evaluates the node and updates the stack states. 
    template <class T>		
    void NodeVariable<T>::evaluate(const Data& data, Stacks& stack)
    {
        stack.push<T>(data.X.row(loc).template cast<T>());
    }

    /// Evaluates the node symbolically
    template <class T>
    void NodeVariable<T>::eval_eqn(Stacks& stack)
    {
        stack.push<T>(name);
    }

    template <class T>
    NodeVariable<T>* NodeVariable<T>::clone_impl() const { return new NodeVariable<T>(*this); }
      
    // rnd_clone is just clone_impl() for variable, since rand vars not supported
    template <class T>
    NodeVariable<T>* NodeVariable<T>::rnd_clone_impl() const { return new NodeVariable<T>(*this); }
    
    template class NodeVariable<bool>;
    template class NodeVariable<int>;
    template class NodeVariable<double>;
}
