all:main

.PHONY:main

main:
	ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk

collector:
	g++ -std=c++14 collector.cpp connection.cpp group.cpp tool.cpp ./codec.cpp ./secure.cpp ../db/mysql-connector.cpp -I.. -I../db/ -lpthread -lmysqlcppconn -o collector
