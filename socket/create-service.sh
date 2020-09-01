#!/bin/bash

set -x

[ $1 ] && [ -f $1 ] || exit 1
chmod +x $1
RUNNABLE=$(realpath $1)
BASE=$(basename $RUNNABLE)
shift

cat >$BASE.service <<EOF
[Unit]
Description=$BASE
After=network.target

[Service]
Type=simple
User=$USER

PIDFile=/tmp/$BASE.pid
ExecStart=$RUNNABLE $*

[Install]
WantedBy=multi-user.target
EOF

sed -i 's/\s\+$//g' $BASE.service
