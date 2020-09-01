all:main

define form-obj
@[ -d $(@D) ] || mkdir -p $(@D)
$(CXX) -c $(CFLAGS) $< -o $@
endef

ROOT_DIR:=$(shell pwd)
OBJ_DIR:=obj
INCS_DIR:=. .. ../db ../io
INCS:=$(foreach i,$(INCS_DIR),$(join -I,$(i)))
SRCS:=collector.cpp\
	connection.cpp\
	group.cpp\
	tool.cpp\
	codec.cpp\
	secure.cpp\
	logger.cpp\
	../db/mysql-connector.cpp
SRCS:=$(addprefix $(ROOT_DIR)/,$(SRCS))
OBJS:=$(addprefix $(OBJ_DIR),$(SRCS:%.cpp=%.o))

CXX?=g++
CFLAGS+=-std=c++14 -Wall $(INCS)
LDLIBS+=-lpthread -lmysqlcppconn

.PHONY:main,clean

$(OBJS):$(OBJ_DIR)%.o:%.cpp
	$(form-obj)

main:
	ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk

collector:$(OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LDLIBS)

clean:
	@rm -f ./collector
	@rm -rf ./obj/
