//
// Created by Steve on 4/24/2020.
//

#ifndef C4FUN_MACRO_H
#define C4FUN_MACRO_H

#define MAIN() \
int main()

#define MAIN_EX(argc, argv) \
int main(int argc, char *argv[])

#define ERROR_RETURN(error, code) \
if(error){\
    return code;\
}

#include <stdio.h>
#include <stdlib.h>

#define ERROR_EXIT(error, msg) \
if(error){\
    perror(msg);\
    exit(EXIT_FAILURE);\
}

#endif //C4FUN_MACRO_H
