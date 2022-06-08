# Linux

## 杀掉进程的几种方法

1.不影响当前终端的其他进程

```
ps -ef | grep xxxxxx | grep -v grep | awk '{print $2}' | xargs kill -9
```

2.当前终端直接所有进程全部杀掉

```
kill -9 $(ps -ef | grep xxxxx)
```



## 深入Linux C/C++ Timer定时器的实现核心原理

[深入Linux C/C++ Timer定时器的实现核心原理 - 云+社区 - 腾讯云 (tencent.com)](https://cloud.tencent.com/developer/article/1763594)
