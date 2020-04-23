#!/bin/bash

main() {
  case $1 in
  0 | sync)
    rsync -avr win:~/github/cpp4fun/multicast/* .
    ;;
  esac
}
main "$*"
