#!/bin/bash

main() {
  set -x
  case ${1:-0} in
  0 | sync)
    rsync -avr win:~/github/cpp4fun/multicast/* .
    ;;
  esac
}
main "$*"
