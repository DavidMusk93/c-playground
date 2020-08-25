//
// Created by Steve on 8/20/2020.
//

#include <string.h>
#include <stdlib.h>

#define CMD_BUFFER_MAX_LEN 2 /*test mode*/
#define CMD_BUFFER struct cmd_buffer_t
CMD_BUFFER{
    void*cmds[CMD_BUFFER_MAX_LEN];
    int nr,nw;
};

typedef void (FN_cmd_buffer_cmd_destroy)(void*cmd);

CMD_BUFFER*cmd_buffer_new(){
    CMD_BUFFER t={.cmds={},.nr=0,.nw=0,};
    CMD_BUFFER*buffer=malloc(sizeof(CMD_BUFFER));
    *buffer=t;
    return buffer;
}
void cmd_buffer_destroy(CMD_BUFFER*self){
    if(self){
        memset(self,0,sizeof(CMD_BUFFER));
        free(self);
    }
}
#define MOVE_FORWARD(x,N) \
do{\
    if(++x==N){\
        x=0;\
    }\
}while(0)
void cmd_buffer_offer(CMD_BUFFER*self,void*cmd,FN_cmd_buffer_cmd_destroy*destroy){ /*may overwrite*/
    void**p=&self->cmds[self->nw];
    if(destroy&&*p){
        destroy(*p);
    }
    *p=cmd; //write first
    MOVE_FORWARD(self->nw,CMD_BUFFER_MAX_LEN);
    if(self->nr==self->nw){
        MOVE_FORWARD(self->nr,CMD_BUFFER_MAX_LEN);
    }
}
void*cmd_buffer_poll(CMD_BUFFER*self){ /*transfer ownership*/
    void*cmd,**p=&self->cmds[self->nr];
    if(!*p){
        return 0;
    }
    cmd=*p,*p=0;
    MOVE_FORWARD(self->nr,CMD_BUFFER_MAX_LEN);
    return cmd;
}
#undef MOVE_FORWARD

#include "macro.h"

MAIN(){
    int data[]={1,2,3,4,5,};
    CMD_BUFFER*buffer=cmd_buffer_new();
    for(int i=0;i<dimension_of(data);++i){
        cmd_buffer_offer(buffer,&data[i],0);
    }
    void*p;
    while((p=cmd_buffer_poll(buffer))){
        LOG("%d",*(int*)p);
    }
    cmd_buffer_destroy(buffer);
}