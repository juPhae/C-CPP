
#ifndef __GET_HOSTINFO_HPP__
#define __GET_HOSTINFO_HPP__
#include <arpa/inet.h> /* inet_ntop */
#include <netdb.h>     /* struct hostent */
#include <unistd.h>    /* gethostname */
#include <iostream>    /* cout */

//使用ifaddrs结构体时需要用到该头文件
#include <ifaddrs.h>

#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/socket.h>
#include <net/if.h>

#define RUN_SUCCESS 0
#define RUN_FAIL -1
//获取ip
int getLocalIpByIfaddrs(std::string& strip) {
    struct ifaddrs* ifAddrStruct = NULL;
    void* tmpAddrPtr = NULL;
    int status = RUN_FAIL;
    char* str_ip = (char*)malloc(INET_ADDRSTRLEN);
    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL) {
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET)  // check it is IP4
        {
            tmpAddrPtr = &((struct sockaddr_in*)ifAddrStruct->ifa_addr)->sin_addr;
            if (inet_ntop(AF_INET, tmpAddrPtr, str_ip, INET_ADDRSTRLEN)) {
                status = RUN_SUCCESS;
                if (strcmp("127.0.0.1", str_ip)) {
                    break;
                }
            }
        } else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6) {
            //可以添加IP6相应代码
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
    strip = str_ip;
    // 释放内存
    free(str_ip);
    str_ip = NULL;
    return status;
}

bool GetHostInfo(std::string& hostName, std::string& Ip) {
    char name[256];
    gethostname(name, sizeof(name));
    hostName = name;

    // char* localip = (char*)malloc(INET_ADDRSTRLEN);
    // if (getLocalIpByIfaddrs(localip) == RUN_SUCCESS) {
    //     printf("get local ip successfully.\n");

    // } else {
    //     printf("Description Failed to obtain the local ip address.\n");
    // }

    (getLocalIpByIfaddrs(Ip) == RUN_SUCCESS) ? (printf("get local ip successfully.\n"))
                                             : (printf("Description Failed to obtain the local ip address.\n"));

    // Ip = localip;

    return true;
}
#endif  //__GET_HOSTINFO_HPP__