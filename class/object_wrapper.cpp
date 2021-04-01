#include "macro.h"

template<class T, class U>
class ObjectWrapper {
public:
    T *GetPtr() {
        return (T *) data_;
    }

private:
    char data_[sizeof(T) + sizeof(U)];
};