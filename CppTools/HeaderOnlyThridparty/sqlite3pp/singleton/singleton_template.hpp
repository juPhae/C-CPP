#pragma once

template <typename T>
class singleton
{
public:
	singleton() = delete;
	virtual ~singleton() = delete;
	singleton(const singleton&) = delete;
	singleton& operator=(const singleton&) = delete;

	template<typename... Args>
	static T& get_instance(Args&&... args)
	{
	    // C++11保证单例的线程安全
		static T t{ std::forward<Args>(args)... };
		return t;
	}
};

#define DEFINE_SINGLETON(class_name) \
public: \
friend class singleton<class_name>; \
using singleton = singleton<class_name>; \
private: \
virtual ~class_name() {} \
class_name(const class_name&) = delete; \
class_name& operator=(const class_name&) = delete; \
public:




/*

使用
#include <iostream>
#include <string>
#include "singleton.h"

class test
{
    // 只需要加入一句代码，就可以将test类变为单例类
	DEFINE_SINGLETON(test);
public:
	test() = default;
	void print()
	{
		std::cout << "Hello world" << std::endl;
	}
};

class test2
{
    // 只需要加入一句代码，就可以将test2类变为单例类
	DEFINE_SINGLETON(test2);
public:
	test2(const std::string& str) : str_(str) {}

	void print()
	{
		std::cout << str_ << std::endl;
	}

private:
	std::string str_;
};

int main()
{
    // 没有参数的单例
	test::singleton::get_instance().print();
	// 带有参数的单例
	test2::singleton::get_instance("nihao").print();
	return 0;
}

*/
