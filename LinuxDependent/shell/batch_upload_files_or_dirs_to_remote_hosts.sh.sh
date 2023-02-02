#!/bin/bash

# 设置用户名和密码
PASSWORD="passwd"
USERNAME="username"

# 设置主机和端口数组
HOSTS_PORTS=(host1:port1 host2:port2)

# 设置文件/目录的本地路径
LOCAL_FILE_PATH="path/to/file_or_dir"

# 设置文件/目录的远程路径
REMOTE_FILE_PATH="path/to/remote_dir"

# 上传文件/目录到远程主机
function upload_to_remote_server {
  local file_path=$1
  local remote_path=$2

  # 判断文件/目录是否存在
  if [ -f $file_path ]; then
    local options="-o StrictHostKeyChecking=no -P"
  elif [ -d $file_path ]; then
    local options="-o StrictHostKeyChecking=no -r -P"
  else
    echo "Error: $file_path is not a file or directory!"
    return 1
  fi

  # 遍历主机和端口数组
  for host_port in "${HOSTS_PORTS[@]}"; do
    (
      host=$(echo $host_port | cut -d ':' -f1)
      port=$(echo $host_port | cut -d ':' -f2)

      # 使用 sshpass 命令上传文件/目录
      sshpass -p $PASSWORD scp $options $port $file_path $USERNAME@$host:$remote_path

      # 判断是否上传成功
      if [ $? -eq 0 ]; then
        echo "Upload $(basename $file_path) to $host:$port successfully!"
      else
        echo "Upload $(basename $file_path) to $host:$port failed!"
        continue
      fi
    ) &
  done
  # 等待所有子进程结束
  wait
}

# 执行上传
upload_to_remote_server $LOCAL_FILE_PATH $REMOTE_FILE_PATH
