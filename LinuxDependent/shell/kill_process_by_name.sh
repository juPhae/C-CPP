#!/bin/bash

#设置需要杀死的进程名
PROCESS_NAME="process_name"

#检索进程是否存在，如果存在就杀掉
if ps -ef | grep "$PROCESS_NAME" | grep -v grep; then
  kill $(ps -ef | grep "$PROCESS_NAME" | grep -v grep | awk '{print $2}')
  echo "$PROCESS_NAME killed successfully!"
else
  echo "$PROCESS_NAME not found, no need to kill."
fi
