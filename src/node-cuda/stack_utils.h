/* FEAT
copyright 2017 William La Cava
license: GNU/GPL v3
*/
#ifndef STACK_UTILS_H
#define STACK_UTILS_H


//stack utils
#include<stddef.h>
namespace FT{
    void dev_allocate(float *& f, size_t Sizef,
                      int *& c, size_t Sizec,
                      bool *& b, size_t Sizeb);
                      
    void copy_from_device(float * dev_f, float * host_f, size_t Sizef,
                          int * dev_c, int * host_c, size_t Sizec,
                          bool * dev_b, bool * host_b, size_t Sizeb);
                          
    void free_device(float * dev_f, int * dev_c, bool * dev_b);
}
#endif
