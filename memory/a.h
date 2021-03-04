#ifndef C_PLAYGROUND_A_H
#define C_PLAYGROUND_A_H

#include <string>

/*@BAD_DESIGN it is danger to declare non POD variables in header*/
const std::string a = "hi,there!";

#endif //C_PLAYGROUND_A_H
