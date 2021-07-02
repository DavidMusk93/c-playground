#!/bin/bash

initialize() {
    LOCALDIR=/home/sun/Documents/github/c-playground/socket/monitor/
    REMOTEDIR=/root/sync/c-playground/socket/monitor/
    REMOTESCRIPTDIR=/opt
    REMOTE=x1
    BUILDDIR=cmake-build-debug
    self=$(basename $0)
    exe=listener_server
}

sync() {
    rsync -avr $LOCALDIR $REMOTE:$REMOTEDIR
    scp $0 $REMOTE:$REMOTESCRIPTDIR
}

build() {
    rm * -rf && cmake .. && make "$1"
}

run() {
    set +e
    pkill $exe
    #    nohup $* &>/tmp/"$(basename $1)".log &
}

main() {
    set -x
    initialize
    case $1 in
    trivial)
        echo "test"
        ;;
    server)
        sync
        #ssh $REMOTE bash $REMOTESCRIPTDIR/$self server-details
        pdsh -R exec -w $REMOTE ssh %h bash $REMOTESCRIPTDIR/$self server-details
        ;;
    server-details)
        set -e
        cd $REMOTEDIR/$BUILDDIR
        build $exe && run ./$exe 8000
        ;;
    esac
}
main "$*"
