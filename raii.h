//
// Created by Steve on 11/16/2020.
//

#ifndef C_PLAYGROUND_RAII_H
#define C_PLAYGROUND_RAII_H

#include <functional>

namespace sun{
    using Closure=std::function<void()>;
    class Defer{
    public:
        Defer(Closure&&closure){
            closure_.swap(closure);
        }
        ~Defer(){
            if(closure_){
                closure_();
            }
        }
        void reset(){
            if(closure_){
                Closure trivial;
                closure_.swap(trivial);
            }
        }
    private:
        Closure closure_;
    };
}

#endif //C_PLAYGROUND_RAII_H
