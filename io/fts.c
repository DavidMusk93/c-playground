//
// Created by Steve on 9/2/2020.
// @ref https://stackoverflow.com/questions/12763906/unexpected-results-using-fts-children-in-c
//

#include "macro.h"

#include <fts.h>

void file_ls(FTS*file_system/*,const char*file_name,int*flags*/){
    FTSENT*node=fts_children(file_system,0);
    if(errno){
        perror("fts_children");
    }
    while(node){
        LOG("found:%s%s",node->fts_path,node->fts_name);
        node=node->fts_link;
    }
    LOG("******\n");
}

int compare(const FTSENT**one,const FTSENT**other){
    return (strcmp((*one)->fts_name,(*other)->fts_name));
}

MAIN_EX(argc,argv){
    FTS*file_system=0;
    FTSENT*node=0;
    if(argc<2){
        LOG("Usage:%s <path-spec>",argv[0]);
        exit(1);
    }
    char in[128]={};
    strcpy(in,argv[1]);
    char*const path[]={in,0};
    file_system=fts_open(path,FTS_COMFOLLOW|FTS_NOCHDIR,&compare);
    if(file_system){
        file_ls(file_system);
        while((node=fts_read(file_system))){
            file_ls(file_system);
        }
        fts_close(file_system);
    }
    return 0;
}