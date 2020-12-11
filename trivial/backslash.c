//
// Created by Steve on 11/1/2020.
//

#include "macro.h"

#include <assert.h>

MAIN(){
#define CVAL(x) ((x)>='0'&&(x)<='9'?(x)-'0':'\177'/*DEL?*/)
    LOG("%d,%d",CVAL('1'),CVAL('a'));
    assert('\177'==0x7f);
}