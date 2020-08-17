//
// Created by Steve on 8/13/2020.
//

#include "../time/now.h"
#include "macro.h"

#include <unistd.h>

MAIN(){
    char *line=0;
    size_t len=0;
    size_t count=0;
    while(getline(&line,&len,stdin)!=-1){
        printf("%s #%zu %s",DEFAULT_TIME_FORMAT(now(0)),++count,line);
    }
    free(line);
}
