# 宏

## 宏的定义

宏（英语：Macro）是一种批量处理的称谓。
计算机科学里的宏是一种抽象（Abstraction），它根据一系列预定义的规则替换一定的文本模式。解释器或编译器在遇到宏时会自动进行这一模式替换。对于编译语言，宏展开在编译时发生，进行宏展开的工具常被称为宏展开器。
宏这一术语也常常被用于许多类似的环境中，它们是源自宏展开的概念，这包括键盘宏和宏语言。绝大多数情况下，“宏”这个词的使用暗示着将小命令或动作转化为一系列指令。
——摘自：百度百科

## 操作系统标识宏

**查看gcc所定义的宏的命令**:

```
#命令一: 
cpp -dM /dev/null
#命令二: 
gcc -dM -E - < /dev/null
```

**1:OS MACRO**:

```
 WINDOWS: _WIN32、WIN32;
 UNIX/LINUX: unix、__unix、__unix__;
 SunOS/SOLARIS: __SVR4、__svr4__、sun、__sun、__sun__、sparc、__sparc、__sparc__;
 HPUX: __hppa、__hppa__、__hpux、__hpux__、_HPUX_SOURCE;
 AIX: _AIX、_AIX32、_AIX41、_AIX43、_AIX51、_AIX52;
 LINUX: linux、__linux、__linux__、__gnu_linux__;
 CPU: __x86_64、__x86_64__(Intel); __amd64、__amd64__(AMD); sparc、__sparc、__sparc__(Sun-SPARC);
```

**2:Compiler MACRO**:

```
 __STDC__: ANSI C标志,值为1,说明兼容ANSI C标准;
 __GNUC__: 它是gcc编译器编译代码时预定义的一个宏;需要针对gcc编写代码时,可以使用这个宏进行条件编译;__GNUC__的值表示gcc的版本号,需要针对特定版本的gcc编写代码时,也可以使用这个宏进行条件编译;__GNUC__的类型是int;
 __GNUC_MINOR__: gcc的次版本号;
 __GNUC_PATCHLEVEL__: gcc的修订号; 比如:gcc3.4.6版本,则__GNUC__=3;__GNUC_MINOR__=4;__GNUC_PATCHLEVEL__=6;
 __GLIBC__: glib的主版本号;
 __GLIBC_MINOR__: glib的次版本号;
```

**3:常量宏**:

```
 __PRETTY_FUNCTION__: 记录当前函数的函数头信息;
 __FUNCTION__: 仅记录当前函数名;
 __VA_ARGS__: 保存了可变参数列表"...";

__PRETTY_FUNCTION__与__FUNCTION__的区别：

__PRETTY_FUNCTION__对函数的打印是函数名带上参数和返回值类型，__FUNCTION__只会打印函数名。
```

**4.C/C++原文件扩展名**:

```
 .C:用C语言编写的源代码文件;
 .CPP/.CXX:用C++语言编写的源代码文件;
 .H/.HPP/.HXX:用C/C++语言编写的头文件;
```

**5.main函数的四种写法**:

```
 int main(void)
 int main(int argc)
 int main(int argc, char** argv)
 int main(int argc, char** argv, char** env)
```

**6.VC++的版本号宏**:

```
Visual Studio 5.0 --> VC++5.0 _MSC_VER = 1100
Visual Studio 6.0 --> VC++6.0 _MSC_VER = 1200
Visual Studio 2002 --> VC++7.0 _MSC_VER = 1300
Visual Studio 2003 --> VC++7.1 _MSC_VER = 1310
Visual Studio 2005 --> VC++8.0 _MSC_VER = 1400
Visual Studio 2008 --> VC++9.0 _MSC_VER = 1500
```



## 系统的宏测试

```

void system_macro() {
    int len = sizeof(int) * 8;
    printf("sizeof(int)=%d\n", len);

    len = sizeof(int*) * 8;
    printf("sizeof(int*)=%d\n", len);

#ifdef _MSC_VER
    printf("_MSC_VER is defined\n");
#endif

#ifdef __GNUC__
    printf("__GNUC__ is defined\n");
#endif

#ifdef __INTEL__
    printf("__INTEL__  is defined\n");
#endif

#ifdef __i386__
    printf("__i386__  is defined\n");
#endif

#ifdef __x86_64__
    printf("__x86_64__  is defined\n");
#endif

#ifdef _WIN32
    printf("_WIN32 is defined\n");
#endif

#ifdef _WIN64
    printf("_WIN64 is defined\n");
#endif


#ifdef __linux__
    printf("__linux__ is defined\n");
#endif

#ifdef __LP64__
    printf("__LP64__ is defined\n");
#endif


#ifdef __amd64
    printf("__amd64 is defined\n");
#endif

#ifdef __SIZEOF_POINTER__
    printf("__SIZEOF_POINTER__ is defined\n");
#endif

#if __SIZEOF_POINTER__ == 2
    /* 16位系统 */
    printf("16位系统\n");
#elif __SIZEOF_POINTER__ == 4
    /* 32位系统 */
    printf("32位系统\n");
#elif __SIZEOF_POINTER__ == 8
    /* 64位系统 */
    printf("64位系统\n");
#endif

}
```



## C++中宏的用途

### 1.定义常量

#### 语法

```cpp
#define CONSTANT_NAME CONSTANT_VALUE
```

举个例子:

```cpp
#define PI 3.1415926
```

#### 替代方案

```cpp
const double PI = 3.1415926
```

两种方法的利弊：
使用宏#define：使用方便快捷，且有许多方法。但是不是类型安全的。
使用const：语法比较长，但是类型安全。
可以自行挑选。

##### 补充内容

可以使用 `#ifdef` (#if defined) 或 `#ifndef` (#if not defined) 来检查常量是否有被定义。
可以使用`#undef`来解除对一个常量的定义。
例如：

```cpp
#ifdef PI
//do sth.
#endif

#ifndef PI
//do sth.
#endif

#undef PI //PI is not defined since then!
```

以上内容仅适用于#define宏，不适用于const。

### 2.多重包含防范

#### 语法&示例

```cpp
#ifndef MAIN_H
#define MAIN_H

#endif
```

#### 用途

保证一个头文件不被多次包含，如例子中是`main.h`的方法，一般宏名起作文件名的大写，.等符号用_替换。

#### 替代方案

```cpp
#pragma once
```

这样与上面效果完全相同。

### 3.创建宏函数

#### 语法

```cpp
#define FUNCTION_NAME(PARAMETER LIST) somefunctions();\
	otherfunctions();
```

宏函数，是宏中难度较高的一部分了。
不太容易理解，而且很容易犯错。

#### 示例

```cpp
#define MAX_THREE(a,b,c) (((a)>(b) ? \
	((a)>(c)?:(a):(c)):\
	((b)>(c)?:(b):(c))))
```

如果调用它（如：`cout << MAX_THREE(1,x,2+3 << endl;`），实际上编译器是进行了如下的操作：

```cpp
cout << (((1)>(x)?((1)>(2+3)?:(1):(2+3)):((x)>(2+3)?:(x):(2+3)))) << endl;
```



#### 语法注意事项

(1) `\`：宏函数可以通过`\`来扩展到下一行，最后一行不用加。适当使用`\`可以增强代码的可读性。
(2)`MAX_THREE`：宏的命名（按照惯例）应该全部大写，单词之间应该用下划线分隔开。宏名应该需要具有自我描述性。
(3)`MAX_THREE(a,b,c)`：在形参与宏名这边一定不！要！加！空！格！！！否则99.99%出错！（否则编译器会识别为常量等，导致错乱）
(4)`(a)(b)(c)`：在宏中使用参数时，两边一定要加括号！当传入的参数是一个数或变量时还好，若传入的是一个表达式，则在展开时会发生重大问题！
(5)整个表达式两边也要加括号！

#### 替代方案

宏函数，最好的替代方案是模板（最新的ANSI C++标准全面支持）

