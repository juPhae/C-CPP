
/*********************************************
 * 
 * https://github.com/yc-liu-123/logging/
 * 
 ********************************************/

#ifndef _YCLOGGING_HPP_
#define _YCLOGGING_HPP_

//切割__FILE__宏的绝对路径，只保留一个源文件名称
#define _NEW_FILE_ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)  


#ifndef MYTHREAD_HPP_
#define MYTHREAD_HPP_

#include <pthread.h>
#include <iostream>
#include <cstring>


class Thread {
	pthread_t thread_;
	bool detached_;
	bool joinable_;
public:
	Thread(void* (*func)(void*), void *arg, bool detach = false, int stack_size = 16383, void* stack_addr = NULL):
		thread_(), detached_(detach), joinable_(!detach) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		detached_ ? 
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) : 
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		if (stack_size > 16383 && stack_addr != NULL) {
			if (pthread_attr_setstack(&attr, stack_addr, stack_size) != 0)
				threadError("fail to set stack size", true);
		}
		else if (stack_size > 16383) {
			if (pthread_attr_setstacksize(&attr, stack_size) != 0)
				threadError("fail to set stack size", true);
		}
		pthread_create(&thread_, &attr, func, arg);
		pthread_attr_destroy(&attr);
	}
	~Thread() {
		if (joinable_) {
			threadError("thread unjoined");
			join();
		}
	}
	bool operator==(Thread& rhs) { return pthread_equal(thread_, (rhs.thread_)); }
	void join(void* res_ptr = nullptr) {
		if (!joinable_)
			threadError("thread can not be joined", true);
		int iRet = pthread_join(thread_, &res_ptr);
		if (iRet != 0)
			res_ptr = 0;
		else
			joinable_ = false;
	}
	void detach() {
		if (detached_)
			threadError("thread has been detached");
		detached_ = true;
		joinable_ = false;
		int iRet = pthread_detach(thread_);
		if (iRet != 0)
			threadError("error happen in detaching thread");
	}
	bool joinable() { return joinable_; }
	bool getdetachstate() { return detached_; }
	pthread_t getID() { return thread_; }
private:
	Thread(Thread& other) {}
	Thread& operator=(Thread& rhs) { return *this; }
	void threadError(const char* msg = "", bool throw_ = false) {
		std::cerr << msg << std::endl;
		if (throw_)
			throw std::runtime_error(msg);
	}
};

class Mutex {
	friend class Condvar;
public:
	Mutex(): mutex_() {
		if (pthread_mutex_init(&mutex_, NULL) != 0)
			mutexError("fail to create mutex", true);
	}
	~Mutex() {
		pthread_mutex_destroy(&mutex_);
	}
	int lock(bool block = true) {
		return block ? pthread_mutex_lock(&mutex_) : pthread_mutex_trylock(&mutex_);
	}
	int unlock() {
		return pthread_mutex_unlock(&mutex_);
	}
private:
	pthread_mutex_t mutex_;
	Mutex(Mutex& other) {}
	Mutex& operator=(Mutex& rhs) { return *this; }
	void mutexError(const char* msg = "", bool throw_ = false) {
		std::cerr << msg << std::endl;
		if (throw_)
			throw std::runtime_error(msg);
	}
};

class Condvar {
public:
	Condvar(): cond_(PTHREAD_COND_INITIALIZER) {}
	~Condvar() { pthread_cond_destroy(&cond_); }
	void wait(Mutex& mu) {
		pthread_cond_wait(&cond_, &(mu.mutex_));
	}
	void signal() {
		pthread_cond_signal(&cond_);
	}
	void boardcast() {
		pthread_cond_broadcast(&cond_);
	}
private:
	pthread_cond_t cond_;
	Condvar(Condvar& other) {}
	Condvar& operator=(Condvar& rhs) { return *this; }
};

class lock_guard {
public:
	lock_guard(Mutex& mutex): myMutex(&mutex) { myMutex->lock(); }
	~lock_guard() { myMutex->unlock(); }
private:
	Mutex* myMutex;
};

#endif // !MYTHREAD

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <ctime>

struct UTC_timer {
    UTC_timer() : past(0) {
        struct timespec tw;
        clock_gettime(CLOCK_MONOTONIC, &tw);
        base_sec = tw.tv_sec;
        base_nsec = tw.tv_nsec;
        renew();
    }
    void get_curr_time(double& pass) {
        double tmp_past(past), tmp_rel_past(rel_past);
        struct timespec tw;
        clock_gettime(CLOCK_MONOTONIC, &tw);
        double cur_past((tw.tv_sec + tw.tv_nsec * 1e-9) - (base_sec + base_nsec * 1e-9));
        tmp_past += cur_past;
        pass = tmp_past;
        tmp_rel_past += cur_past;
        int int_past(tmp_rel_past);
        if (int_past > 0) {
            int m(incr(&sec, int_past, 60));
            tmp_rel_past -= int_past;
            if (m > 0)
                renew();
            else
                reset_utc_fmt_sufix();
        }
        base_nsec = tw.tv_nsec;
        base_sec = tw.tv_sec;
        rel_past = tmp_rel_past;
        past = tmp_past;
    }
    double past;
    char utc_fmt[20];
private:
    void renew() {
        time_t t = time(NULL);
        struct tm cur_tm;
        localtime_r(&t, &cur_tm);
        year = cur_tm.tm_year + 1900;
        mon = cur_tm.tm_mon + 1;
        day = cur_tm.tm_mday;
        hr = cur_tm.tm_hour;
        min = cur_tm.tm_min;
        sec = cur_tm.tm_sec;
        rel_past = 0;
        reset_utc_fmt();
    }
    int incr(unsigned int* orig, const int passed, const int itv) {
        int w(*orig + passed);
        *orig = w % itv;
        return w / itv;
    }
    void reset_utc_fmt() {
        snprintf(utc_fmt, 20, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", year, mon, day, hr, min, sec);
    }
    void reset_utc_fmt_sufix() {
        snprintf(utc_fmt + 17, 3, "%.2d", sec);
    }
    unsigned int year, mon, day, hr, min, sec;
    uint64_t base_sec, base_nsec;
    double rel_past;
};

#endif //TIMER_HPP_

#ifndef HANDLER_HPP_
#define HANDLER_HPP_

#include <cstdio>

namespace Logging {

	class Handler {
		Handler(Handler& rhs) {}
		Handler& operator=(Handler& rhs) { return *this; }
	public:
		explicit Handler() {}
		virtual ~Handler() {}
		virtual void write(const char* text, const size_t len) {
			printf("%.*s", len, text);
		}
	};

	class fileHandler : public Handler {
		fileHandler(fileHandler& rhs) {}
		fileHandler& operator=(fileHandler& rhs) { return *this; }
	public:
		explicit fileHandler(const char* name, const char* mode = "w") : file_(fopen(name, mode)) {
			if (!file_)
				throw std::runtime_error("can not create output file");
		}
		~fileHandler() { fclose(file_); }
		void write(const char* text, const size_t len) {
			fwrite(text, sizeof(char), len, file_);
		}
	private:
		FILE* file_;
	};
}

#endif  //HANDLER_HPP_

#ifndef MYLOGGING_HPP_
#define MYLOGGING_HPP_


#include <cstring>

namespace Logging {

	enum levels { CRITICAL = 50, ERROR = 40, WARNING = 30, INFO = 20, DEBUG = 10, VERBOSE = 0 };

	void* writer(void*);

	class Records {
		static Records* Recs;
		static Mutex Lock_;

		friend void* writer(void*);

		Records();
	public:
		static const unsigned int INIT_SIZE = 8;
		static const unsigned int TEXT_SIZE = 1024 * 1024;
		static const unsigned int SLOT_SIZE = 1024;

		static Records& instance();
		void input(const char*, const char*, const char*, const unsigned int);
		void flush();

	private:

		class Cell {
		public:
			Cell* next_;
			int wout_;
			bool full_;
			unsigned int cur_pos;

			Cell(Cell* next = nullptr) :
				next_(next), full_(false), text_(""), cur_pos(0), wout_(-1) {}
			void input(const char*, const unsigned int, const unsigned int);
			void output();

		private:
			char text_[TEXT_SIZE];
		};

		UTC_timer timer_;
		unsigned int num_;
		Cell* in_, * out_;
		Mutex record_, sig_;
		Condvar cond_;
		bool ready_, writing_;
		Thread daemon_;
	};

	struct Logger {
		Logger(const int level, const unsigned int out, const char* source): 
			level_(level), out_(out), source_(new char[strlen(source) + 1]) {
			strcpy(source_, source);
		}
		~Logger() { delete [] source_; }
		char* source_;
		unsigned int out_;
		int level_;
	};
}

class logging {
	static logging* man;
	static Mutex Lock_;
	static UTC_timer timer_;
	static const int num_ = 10;
	int use_log, free_log, free_han;
	Mutex llock, hlock;
	bool blocking_;
	Logging::Logger* logMap[num_];
	Logging::Handler* handleMap[num_ + 1];
public:
	static logging& instance();
	static int config(const char* name = "main", const int level = Logging::INFO, const char* header = "",
		Logging::Handler* han = nullptr);
	static int close(unsigned int idx);
	static Logging::Logger* getlogger(const unsigned int);
	static Logging::Handler* gethandle(const unsigned int);
	static void flush();
	static void verbose(const char* msg, const unsigned int log_idx = 0);
	static void verbose(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void debug(const char* msg, const unsigned int log_idx = 0);
	static void debug(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void info(const char* msg, const unsigned int log_idx = 0);
	static void info(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void warning(const char* msg, const unsigned int log_idx = 0);
	static void warning(const char* msg, const char* source, const unsigned int log_idx = 0);
	static void critical(const char* msg, const unsigned int log_idx = 0);
	static void critical(const char* msg, const char* source, const unsigned int log_idx = 0);
private:
	logging();
	Logging::Logger* glogger(const unsigned int);
	int createLogger(const char* name, const int level = Logging::INFO, const unsigned int out = 0, const char* header = "");
	int rmLogger(unsigned int);
	int addHandle(Logging::Handler*);
	int rmHandle(unsigned int);
};


//definition of writing function as daemon thread

void* Logging::writer(void* meanless) {
	lock_guard g(Records::instance().sig_);
	Records::instance().ready_ = true;
	while (true) {
		Records::instance().cond_.wait(Records::instance().sig_);
		while (Records::instance().out_->full_) {
			if (Records::instance().out_->cur_pos > 0)
				Records::instance().out_->output();
			Records::instance().out_ = Records::instance().out_->next_;
		}
	}
	return 0;
}

//definition of menber functions in <Logging::Records::Cell>

void Logging::Records::Cell::input(const char* text, const unsigned int len, const unsigned int out)
{
	wout_ = out;
	strncpy(text_ + cur_pos, text, len);
	cur_pos += len;
	if (cur_pos >= TEXT_SIZE - 1)
		full_ = true;
}

void Logging::Records::Cell::output() {
	if (cur_pos > 0) {
		logging::gethandle(wout_)->write(text_, cur_pos);
		wout_ = -1;
		cur_pos = 0;
		full_ = false;
	}
	else
		throw std::runtime_error("empty Cell");
}

//definition of menber functions in <Logging::Records>

Logging::Records& Logging::Records::instance() {
	if (Recs == nullptr) {
		lock_guard guard(Lock_);
		if (Recs == nullptr)
			Recs = new Records();
	}
	return *Recs;
}

void Logging::Records::input(const char* level, const char* source, const char* msg, const unsigned int textout)
{
	double past;
	char text[TEXT_SIZE];
	timer_.get_curr_time(past);
	int len = snprintf(text, Records::TEXT_SIZE, "%s [%s] %.2f [%s]: %s\n", 
		timer_.utc_fmt, level, past, source, msg);
	bool sigw(false);
	record_.lock();
	if ((in_->full_ || TEXT_SIZE - in_->cur_pos - 1 < len) || 
		(in_->wout_ != -1 && textout != in_->wout_)) {
		if (!in_->next_->full_ && num_ < SLOT_SIZE) {
			in_->next_ = new Cell(in_->next_);
			++num_;
		}
		sigw = true;
		if (!(in_->next_->full_)) {
			in_->full_ = true;
			in_ = in_->next_;
			in_->input(text, len, textout);
		}
	}
	else {
		in_->input(text, len, textout);
	}
	record_.unlock();
	if (sigw) {
		if (ready_ && sig_.lock(false) == 0) {
			cond_.signal();
			sig_.unlock();
		}
	}
}

void Logging::Records::flush()
{
	in_->full_ = true;
	while (!ready_);
	while (out_->cur_pos != 0 || out_->cur_pos != 0) {
		if (sig_.lock(false) == 0) {
			cond_.signal();
			sig_.unlock();
		}
	}
}

Logging::Records::Records() : 
	num_(INIT_SIZE), record_(), sig_(), cond_(), timer_(),
	ready_(false), writing_(true), daemon_(writer, (void*)NULL, true) {
	Cell* head(nullptr), * tail(nullptr);
	for (int i = 0; i < num_; ++i) {
		in_ = new Cell(head);
		head = in_;
		if (tail == nullptr)
			tail = in_;
	}
	tail->next_ = head;
	in_ = head;
	out_ = in_;
}

//definition of menber functions in <logging>

logging& logging::instance()
{
	if (man == nullptr) {
		lock_guard guard(Lock_);
		if (man == nullptr)
			man = new logging();
	}
	return *man;
}

int logging::config(const char* name, const int level, const char* header, Logging::Handler* han) {
	instance();
	Logging::Records::instance();
	if (han == nullptr)
		return instance().createLogger(name, level, 0, header);
	else {
		int idx(instance().addHandle(han));
		return instance().createLogger(name, level, idx, header);
	}
}

int logging::close(unsigned int idx)
{
	return instance().rmLogger(idx);
}

Logging::Logger* logging::getlogger(const unsigned int idx)
{
	return instance().glogger(idx);
}

Logging::Handler* logging::gethandle(const unsigned int idx)
{
	if (idx < num_ && instance().handleMap[idx] != nullptr) {
		return instance().handleMap[idx];
	}
	return instance().handleMap[0];
}

void logging::flush()
{
	instance().blocking_ = true;
	Logging::Records::instance().flush();
	instance().blocking_ = false;
}

void logging::verbose(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("VERBOSE", "default", msg, 0);
		else if (lg->level_ <= Logging::VERBOSE)
			Logging::Records::instance().input("VERBOSE", lg->source_, msg, lg->out_);
	}
}

void logging::verbose(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("VERBOSE", source, msg, 0);
		else if (lg->level_ <= Logging::VERBOSE)
			Logging::Records::instance().input("VERBOSE", source, msg, lg->out_);
	}
}

void logging::debug(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("DEBUG", "default", msg, 0);
		else if (lg->level_ <= Logging::DEBUG)
			Logging::Records::instance().input("DEBUG", lg->source_, msg, lg->out_);
	}
}

void logging::debug(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("DEBUG", source, msg, 0);
		else if (lg->level_ <= Logging::DEBUG)
			Logging::Records::instance().input("DEBUG", source, msg, lg->out_);
	}
}

void logging::info(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("INFO", "default", msg, 0);
		else if (lg->level_ <= Logging::INFO)
			Logging::Records::instance().input("INFO", lg->source_, msg, lg->out_);
	}
}

void logging::info(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("INFO", source, msg, 0);
		else if (lg->level_ <= Logging::INFO)
			Logging::Records::instance().input("INFO", source, msg, lg->out_);
	}
}

void logging::warning(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("WARNING", "default", msg, 0);
		else if (lg->level_ <= Logging::WARNING)
			Logging::Records::instance().input("WARNING", lg->source_, msg, lg->out_);
	}
}

void logging::warning(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("WARNING", source, msg, 0);
		else if (lg->level_ <= Logging::WARNING)
			Logging::Records::instance().input("WARNING", source, msg, lg->out_);
	}
}

void logging::critical(const char* msg, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(instance().glogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("CRITICAL", "default", msg, 0);
		else if (lg->level_ <= Logging::CRITICAL)
			Logging::Records::instance().input("CRITICAL", lg->source_, msg, lg->out_);
	}
}

void logging::critical(const char* msg, const char* source, const unsigned int log_idx)
{
	if (!instance().blocking_) {
		Logging::Logger* lg(getlogger(log_idx));
		if (lg == nullptr)
			Logging::Records::instance().input("CRITICAL", source, msg, 0);
		else if (lg->level_ <= Logging::CRITICAL)
			Logging::Records::instance().input("CRITICAL", source, msg, lg->out_);
	}
}

logging::logging() : blocking_(false), use_log(-1), free_log(0), free_han(1), llock(), hlock() {
	Logging::Handler* std_han = new Logging::Handler();
	handleMap[0] = std_han;
}

Logging::Logger* logging::glogger(const unsigned int idx)
{
	while (blocking_);
	if (idx < num_ && logMap[idx] != nullptr) {
		return logMap[idx];
	}
	else if (use_log != -1)
		return logMap[use_log];
	return nullptr;
}

int logging::createLogger(const char* name, const int level, const unsigned int out, const char* header)
{
	if (free_log < num_) {
		Logging::Logger* lg = new Logging::Logger(level, out, name);
		llock.lock();
		int p(free_log);
		instance().logMap[free_log] = lg;
		if (use_log < 0)
			use_log = free_log;
		while (free_log < num_ && instance().logMap[free_log] != nullptr)
			++free_log;
		llock.unlock();
		double past;
		timer_.get_curr_time(past);
		unsigned int len(strlen(name) + 19 + 24);
		char str[len];
		snprintf(str, len + 1, "Log for [%s] created at: %s\n", name, timer_.utc_fmt);
		handleMap[out]->write(str, strlen(str));
		if (strlen(header) > 0) {
			handleMap[out]->write(header, strlen(header));
			handleMap[out]->write("\n", 1);
		}
		return p;
	}
	return -1;
}

int logging::rmLogger(unsigned int idx)
{
	if (idx >= 0 && idx < num_ && logMap[idx] != nullptr) {
		lock_guard g(llock);
		if (idx == use_log) {
			int p(0);
			while (p < num_ && logMap[p] == nullptr && p != idx)
				++p;
			if (p == num_)
				return 0;
			use_log = p;
		}
		if (idx < free_log)
			free_log = idx;
		rmHandle(logMap[idx]->out_);
		delete logMap[idx];
		logMap[idx] = nullptr;
		return 1;
	}
	return 0;
}

int logging::addHandle(Logging::Handler* han)
{
	if (free_han < num_) {
		lock_guard g(hlock);
		int p(free_han);
		instance().handleMap[free_han] = han;
		while (free_han < num_ && instance().handleMap[free_han] != nullptr)
			++free_han;
		return p;
	}
	return -1;
}

int logging::rmHandle(unsigned int idx)
{
	if (idx > 0 && idx < num_ + 1 && handleMap[idx] != nullptr) {
		lock_guard g(hlock);
		delete handleMap[idx];
		handleMap[idx] = nullptr;
		if (idx < free_han)
			free_han = idx;
		return 1;
	}
	return 0;
}

const unsigned int Logging::Records::INIT_SIZE;
const unsigned int Logging::Records::TEXT_SIZE;
const unsigned int Logging::Records::SLOT_SIZE;
Logging::Records* Logging::Records::Recs(nullptr);
Mutex Logging::Records::Lock_;

logging* logging::man(nullptr);
Mutex logging::Lock_;
UTC_timer logging::timer_;
const int logging::num_;

#endif // !LOGGING_HPP_

#endif  //_YCLOGGING_HPP_