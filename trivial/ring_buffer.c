//
// Created by Steve on 7/31/2020.
//

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "macro.h"

struct ring_buffer_t{
    void **data;
    size_t size;
    size_t capacity;
    void *blob;
};
#define RING struct ring_buffer_t

void ring_buffer_init(RING*self,size_t capacity,size_t block){
#define ALIGN(x) (((x)&~7U)+((x)&7U)?8U:0)
    self->blob=malloc(capacity*(sizeof(void*)+ALIGN(block)));
    self->data=self->blob;
    char *p=(char*)self->blob+capacity*sizeof(void*);
    for(size_t i=0;i<capacity;++i){
        self->data[i]=p;
        p+=ALIGN(block);
    }
    self->size=0,self->capacity=capacity;
#undef ALIGN
}

void ring_buffer_rls(RING*self){
    free(self->blob);
    memset(self,0,sizeof(RING));
}

void*ring_buffer_get(RING*self){
    void *p=self->data[self->size];
    if(++self->size==self->capacity){
        self->size=0;
    }
    return p;
}

struct weight_window_t{
    int sum;
    int is_full;
    size_t length;
    RING buffer;
};
#define WINDOW struct weight_window_t

void weight_window_init(WINDOW*self,size_t length){
    self->sum=0;
    self->is_full=0;
    self->length=length;
    ring_buffer_init(&self->buffer,length,sizeof(int));
};

void weight_window_rls(WINDOW*self){
    ring_buffer_rls(&self->buffer);
    memset(self,0,sizeof(WINDOW));
}

bool weight_window_is_full(WINDOW*self){
    return self->is_full;
}

void weight_window_add_weight(WINDOW*self,int weight){
    int *p=ring_buffer_get(&self->buffer);
//    LOG("@%s %p,%d,%d",__func__,p,*p,weight);
    self->sum+=weight;
    if(self->is_full){
        self->sum-=*p;
    }else{
        self->is_full=self->buffer.size==0;
    }
    *p=weight;
}

int weight_window_average(WINDOW*self){
    return self->sum/(int)self->length;
}

MAIN(){
    int data[]={-2055,-2065,-2070,-2070,-2070,-2070,-2070,-2070,-2070,-2070,-2070,};
    WINDOW win;
    weight_window_init(&win,3);
    for(int i=0;i<dimension_of(data);++i){
        weight_window_add_weight(&win,data[i]);
        LOG("%d,%d",weight_window_is_full(&win),win.is_full?weight_window_average(&win):0);
    }
    weight_window_rls(&win);
    return 0;
}