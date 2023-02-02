# 设置用户名和密码
PASSWORD="passwd"
USERNAME="username"

# 设置主机和端口数组
HOSTS_PORTS=(host1:port1 host2:port2)

# 设置要执行的命令
COMMAND="ls"

# 执行命令到远程主机
function run_command_on_remote_server {
  local command=$1

  # 遍历主机和端口数组

  for host_port in "${HOSTS_PORTS[@]}"; do
    (
      host=$(echo $host_port | cut -d ':' -f1)
      port=$(echo $host_port | cut -d ':' -f2)

      # 使用 sshpass 命令连接远程主机并执行命令
      sshpass -p $PASSWORD ssh -o StrictHostKeyChecking=no -p $port $USERNAME@$host $command

      # 判断命令是否执行成功
      if [ $? -eq 0 ]; then
        echo "Run $command on $host:$port successfully!"
      else
        echo "Run $command on $host:$port failed!"
        continue
      fi
    ) &

  done

  # 等待所有子进程结束
  wait
}

# 执行命令
run_command_on_remote_server "$COMMAND"
