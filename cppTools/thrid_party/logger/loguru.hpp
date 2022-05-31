/*
Loguru logging library for C++, by Emil Ernerfeldt.
www.github.com/emilk/loguru
If you find Loguru useful, please let me know on twitter or in a mail!
Twitter: @ernerfeldt
Mail:    emil.ernerfeldt@gmail.com
Website: www.ilikebigbits.com

# License
	This software is in the public domain. Where that dedication is not
	recognized, you are granted a perpetual, irrevocable license to
	copy, modify and distribute it as you see fit.

# Inspiration
	Much of Loguru was inspired by GLOG, https://code.google.com/p/google-glog/.
	The choice of public domain is fully due Sean T. Barrett
	and his wonderful stb libraries at https://github.com/nothings/stb.

# Version history
	* Version 0.1.0 - 2015-03-22 - Works great on Mac.
	* Version 0.2.0 - 2015-09-17 - Removed the only dependency.
	* Version 0.3.0 - 2015-10-02 - Drop-in replacement for most of GLOG
	* Version 0.4.0 - 2015-10-07 - Single-file!
	* Version 0.5.0 - 2015-10-17 - Improved file logging
	* Version 0.6.0 - 2015-10-24 - Add stack traces
	* Version 0.7.0 - 2015-10-27 - Signals
	* Version 0.8.0 - 2015-10-30 - Color logging.
	* Version 0.9.0 - 2015-11-26 - ABORT_S and proper handling of FATAL
	* Version 1.0.0 - 2016-02-14 - ERROR_CONTEXT
	* Version 1.1.0 - 2016-02-19 - -v OFF, -v INFO etc
	* Version 1.1.1 - 2016-02-20 - textprintf vs strprintf
	* Version 1.1.2 - 2016-02-22 - Remove g_alsologtostderr
	* Version 1.1.3 - 2016-02-29 - ERROR_CONTEXT as linked list
	* Version 1.2.0 - 2016-03-19 - Add get_thread_name()
	* Version 1.2.1 - 2016-03-20 - Minor fixes
	* Version 1.2.2 - 2016-03-29 - Fix issues with set_fatal_handler throwing an exception
	* Version 1.2.3 - 2016-05-16 - Log current working directory in loguru::init().
	* Version 1.2.4 - 2016-05-18 - Custom replacement for -v in loguru::init() by bjoernpollex
	* Version 1.2.5 - 2016-05-18 - Add ability to print ERROR_CONTEXT of parent thread.
	* Version 1.2.6 - 2016-05-19 - Bug fix regarding VLOG verbosity argument lacking ().
	* Version 1.2.7 - 2016-05-23 - Fix PATH_MAX problem.
	* Version 1.2.8 - 2016-05-26 - Add shutdown() and remove_all_callbacks()
	* Version 1.2.9 - 2016-06-09 - Use a monotonic clock for uptime.
	* Version 1.3.0 - 2016-07-20 - Fix issues with callback flush/close not being called.
	* Version 1.3.1 - 2016-07-20 - Add LOGURU_UNSAFE_SIGNAL_HANDLER to toggle stacktrace on signals.
	* Version 1.3.2 - 2016-07-20 - Add loguru::arguments()
	* Version 1.4.0 - 2016-09-15 - Semantic versioning + add loguru::create_directories
	* Version 1.4.1 - 2016-09-29 - Customize formating with LOGURU_FILENAME_WIDTH
	* Version 1.5.0 - 2016-12-22 - LOGURU_USE_FMTLIB by kolis and LOGURU_WITH_FILEABS by scinart
	* Version 1.5.1 - 2017-08-08 - Terminal colors on Windows 10 thanks to looki
	* Version 1.6.0 - 2018-01-03 - Add LOGURU_RTTI and LOGURU_STACKTRACES settings
	* Version 1.7.0 - 2018-01-03 - Add ability to turn off the preamble with loguru::g_preamble
	* Version 1.7.1 - 2018-04-05 - Add function get_fatal_handler
	* Version 1.7.2 - 2018-04-22 - Fix a bug where large file names could cause stack corruption (thanks @ccamporesi)
	* Version 1.8.0 - 2018-04-23 - Shorten long file names to keep preamble fixed width
	* Version 1.9.0 - 2018-09-22 - Adjust terminal colors, add LOGURU_VERBOSE_SCOPE_ENDINGS, add LOGURU_SCOPE_TIME_PRECISION, add named log levels
	* Version 2.0.0 - 2018-09-22 - Split loguru.hpp into loguru.hpp and loguru.cpp
	* Version 2.1.0 - 2019-09-23 - Update fmtlib + add option to loguru::init to NOT set main thread name.
	* Version 2.2.0 - 2020-07-31 - Replace LOGURU_CATCH_SIGABRT with struct SignalOptions

# Compiling
	Just include <loguru.hpp> where you want to use Loguru.
	Then, in one .cpp file #include <loguru.cpp>
	Make sure you compile with -std=c++11 -lstdc++ -lpthread -ldl

# Usage
	For details, please see the official documentation at emilk.github.io/loguru

	#include <loguru.hpp>

	int main(int argc, char* argv[]) {
		loguru::init(argc, argv);

		// Put every log message in "everything.log":
		loguru::add_file("everything.log", loguru::Append, loguru::Verbosity_MAX);

		LOG_F(INFO, "The magic number is %d", 42);
	}

*/

#ifndef __LOGURU_HPP__
#define __LOGURU_HPP__

#if defined(LOGURU_IMPLEMENTATION)
	#error "You are defining LOGURU_IMPLEMENTATION. This is for older versions of Loguru. You should now instead include loguru.cpp (or build it and link with it)"
#endif

// Disable all warnings from gcc/clang:
#if defined(__clang__)
	#pragma clang system_header
#elif defined(__GNUC__)
	#pragma GCC system_header
#endif

#ifndef LOGURU_HAS_DECLARED_FORMAT_HEADER
#define LOGURU_HAS_DECLARED_FORMAT_HEADER

// Semantic versioning. Loguru version can be printed with printf("%d.%d.%d", LOGURU_VERSION_MAJOR, LOGURU_VERSION_MINOR, LOGURU_VERSION_PATCH);
#define LOGURU_VERSION_MAJOR 2
#define LOGURU_VERSION_MINOR 1
#define LOGURU_VERSION_PATCH 0

#if defined(_MSC_VER)
#include <sal.h>	// Needed for _In_z_ etc annotations
#endif

#if defined(__linux__) || defined(__APPLE__)
#define LOGURU_SYSLOG 1
#else
#define LOGURU_SYSLOG 0
#endif

// ----------------------------------------------------------------------------

#ifndef LOGURU_EXPORT
	// Define to your project's export declaration if needed for use in a shared library.
	#define LOGURU_EXPORT
#endif

#ifndef LOGURU_SCOPE_TEXT_SIZE
	// Maximum length of text that can be printed by a LOG_SCOPE.
	// This should be long enough to get most things, but short enough not to clutter the stack.
	#define LOGURU_SCOPE_TEXT_SIZE 196
#endif

#ifndef LOGURU_FILENAME_WIDTH
	// Width of the column containing the file name
	#define LOGURU_FILENAME_WIDTH 23
#endif

#ifndef LOGURU_THREADNAME_WIDTH
	// Width of the column containing the thread name
	#define LOGURU_THREADNAME_WIDTH 16
#endif

#ifndef LOGURU_SCOPE_TIME_PRECISION
	// Resolution of scope timers. 3=ms, 6=us, 9=ns
	#define LOGURU_SCOPE_TIME_PRECISION 3
#endif

#ifdef LOGURU_CATCH_SIGABRT
	#error "You are defining LOGURU_CATCH_SIGABRT. This is for older versions of Loguru. You should now instead set the options passed to loguru::init"
#endif

#ifndef LOGURU_VERBOSE_SCOPE_ENDINGS
	// Show milliseconds and scope name at end of scope.
	#define LOGURU_VERBOSE_SCOPE_ENDINGS 1
#endif

#ifndef LOGURU_REDEFINE_ASSERT
	#define LOGURU_REDEFINE_ASSERT 0
#endif

#ifndef LOGURU_WITH_STREAMS
	#define LOGURU_WITH_STREAMS 0
#endif

#ifndef LOGURU_REPLACE_GLOG
	#define LOGURU_REPLACE_GLOG 0
#endif

#if LOGURU_REPLACE_GLOG
	#undef LOGURU_WITH_STREAMS
	#define LOGURU_WITH_STREAMS 1
#endif

#if defined(LOGURU_UNSAFE_SIGNAL_HANDLER)
	#error "You are defining LOGURU_UNSAFE_SIGNAL_HANDLER. This is for older versions of Loguru. You should now instead set the unsafe_signal_handler option when you call loguru::init."
#endif

#if LOGURU_IMPLEMENTATION
	#undef LOGURU_WITH_STREAMS
	#define LOGURU_WITH_STREAMS 1
#endif

#ifndef LOGURU_USE_FMTLIB
	#define LOGURU_USE_FMTLIB 0
#endif

#ifndef LOGURU_USE_LOCALE
        #define LOGURU_USE_LOCALE 0
#endif

#ifndef LOGURU_WITH_FILEABS
	#define LOGURU_WITH_FILEABS 0
#endif

#ifndef LOGURU_RTTI
#if defined(__clang__)
	#if __has_feature(cxx_rtti)
		#define LOGURU_RTTI 1
	#endif
#elif defined(__GNUG__)
	#if defined(__GXX_RTTI)
		#define LOGURU_RTTI 1
	#endif
#elif defined(_MSC_VER)
	#if defined(_CPPRTTI)
		#define LOGURU_RTTI 1
	#endif
#endif
#endif

#ifdef LOGURU_USE_ANONYMOUS_NAMESPACE
	#define LOGURU_ANONYMOUS_NAMESPACE_BEGIN namespace {
	#define LOGURU_ANONYMOUS_NAMESPACE_END }
#else
	#define LOGURU_ANONYMOUS_NAMESPACE_BEGIN
	#define LOGURU_ANONYMOUS_NAMESPACE_END
#endif

// --------------------------------------------------------------------
// Utility macros

#define LOGURU_CONCATENATE_IMPL(s1, s2) s1 ## s2
#define LOGURU_CONCATENATE(s1, s2) LOGURU_CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#   define LOGURU_ANONYMOUS_VARIABLE(str) LOGURU_CONCATENATE(str, __COUNTER__)
#else
#   define LOGURU_ANONYMOUS_VARIABLE(str) LOGURU_CONCATENATE(str, __LINE__)
#endif

#if defined(__clang__) || defined(__GNUC__)
	// Helper macro for declaring functions as having similar signature to printf.
	// This allows the compiler to catch format errors at compile-time.
	#define LOGURU_PRINTF_LIKE(fmtarg, firstvararg) __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
	#define LOGURU_FORMAT_STRING_TYPE const char*
#elif defined(_MSC_VER)
	#define LOGURU_PRINTF_LIKE(fmtarg, firstvararg)
	#define LOGURU_FORMAT_STRING_TYPE _In_z_ _Printf_format_string_ const char*
#else
	#define LOGURU_PRINTF_LIKE(fmtarg, firstvararg)
	#define LOGURU_FORMAT_STRING_TYPE const char*
#endif

// Used to mark log_and_abort for the benefit of the static analyzer and optimizer.
#if defined(_MSC_VER)
#define LOGURU_NORETURN __declspec(noreturn)
#else
#define LOGURU_NORETURN __attribute__((noreturn))
#endif

#if defined(_MSC_VER)
#define LOGURU_PREDICT_FALSE(x) (x)
#define LOGURU_PREDICT_TRUE(x)  (x)
#else
#define LOGURU_PREDICT_FALSE(x) (__builtin_expect(x,     0))
#define LOGURU_PREDICT_TRUE(x)  (__builtin_expect(!!(x), 1))
#endif

#if LOGURU_USE_FMTLIB
	#include <fmt/format.h>
	#define LOGURU_FMT(x) "{:" #x "}"
#else
	#define LOGURU_FMT(x) "%" #x
#endif

#ifdef _WIN32
	#define STRDUP(str) _strdup(str)
#else
	#define STRDUP(str) strdup(str)
#endif

#include <stdarg.h>

// --------------------------------------------------------------------
LOGURU_ANONYMOUS_NAMESPACE_BEGIN

namespace loguru
{
	// Simple RAII ownership of a char*.
	class LOGURU_EXPORT Text
	{
	public:
		explicit Text(char* owned_str) : _str(owned_str) {}
		~Text();
		Text(Text&& t)
		{
			_str = t._str;
			t._str = nullptr;
		}
		Text(Text& t) = delete;
		Text& operator=(Text& t) = delete;
		void operator=(Text&& t) = delete;

		const char* c_str() const { return _str; }
		bool empty() const { return _str == nullptr || *_str == '\0'; }

		char* release()
		{
			auto result = _str;
			_str = nullptr;
			return result;
		}

	private:
		char* _str;
	};

	// Like printf, but returns the formated text.
#if LOGURU_USE_FMTLIB
	LOGURU_EXPORT
	Text vtextprintf(const char* format, fmt::format_args args);

	template<typename... Args>
	LOGURU_EXPORT
	Text textprintf(LOGURU_FORMAT_STRING_TYPE format, const Args&... args) {
		return vtextprintf(format, fmt::make_format_args(args...));
	}
#else
	LOGURU_EXPORT
	Text textprintf(LOGURU_FORMAT_STRING_TYPE format, ...) LOGURU_PRINTF_LIKE(1, 2);
#endif

	// Overloaded for variadic template matching.
	LOGURU_EXPORT
	Text textprintf();

	using Verbosity = int;

#undef FATAL
#undef ERROR
#undef WARNING
#undef INFO
#undef MAX

	enum NamedVerbosity : Verbosity
	{
		// Used to mark an invalid verbosity. Do not log to this level.
		Verbosity_INVALID = -10, // Never do LOG_F(INVALID)

		// You may use Verbosity_OFF on g_stderr_verbosity, but for nothing else!
		Verbosity_OFF     = -9, // Never do LOG_F(OFF)

		// Prefer to use ABORT_F or ABORT_S over LOG_F(FATAL) or LOG_S(FATAL).
		Verbosity_FATAL   = -3,
		Verbosity_ERROR   = -2,
		Verbosity_WARNING = -1,

		// Normal messages. By default written to stderr.
		Verbosity_INFO    =  0,

		// Same as Verbosity_INFO in every way.
		Verbosity_0       =  0,

		// Verbosity levels 1-9 are generally not written to stderr, but are written to file.
		Verbosity_1       = +1,
		Verbosity_2       = +2,
		Verbosity_3       = +3,
		Verbosity_4       = +4,
		Verbosity_5       = +5,
		Verbosity_6       = +6,
		Verbosity_7       = +7,
		Verbosity_8       = +8,
		Verbosity_9       = +9,

		// Do not use higher verbosity levels, as that will make grepping log files harder.
		Verbosity_MAX     = +9,
	};

	struct Message
	{
		// You would generally print a Message by just concatenating the buffers without spacing.
		// Optionally, ignore preamble and indentation.
		Verbosity   verbosity;   // Already part of preamble
		const char* filename;    // Already part of preamble
		unsigned    line;        // Already part of preamble
		const char* preamble;    // Date, time, uptime, thread, file:line, verbosity.
		const char* indentation; // Just a bunch of spacing.
		const char* prefix;      // Assertion failure info goes here (or "").
		const char* message;     // User message goes here.
	};

	/* Everything with a verbosity equal or greater than g_stderr_verbosity will be
	written to stderr. You can set this in code or via the -v argument.
	Set to loguru::Verbosity_OFF to write nothing to stderr.
	Default is 0, i.e. only log ERROR, WARNING and INFO are written to stderr.
	*/
	LOGURU_EXPORT extern Verbosity g_stderr_verbosity;
	LOGURU_EXPORT extern bool      g_colorlogtostderr; // True by default.
	LOGURU_EXPORT extern unsigned  g_flush_interval_ms; // 0 (unbuffered) by default.
	LOGURU_EXPORT extern bool      g_preamble_header; // Prepend each log start by a descriptions line with all columns name? True by default.
	LOGURU_EXPORT extern bool      g_preamble; // Prefix each log line with date, time etc? True by default.

	/* Specify the verbosity used by loguru to log its info messages including the header
	logged when logged::init() is called or on exit. Default is 0 (INFO).
	*/
	LOGURU_EXPORT extern Verbosity g_internal_verbosity;

	// Turn off individual parts of the preamble
	LOGURU_EXPORT extern bool      g_preamble_date; // The date field
	LOGURU_EXPORT extern bool      g_preamble_time; // The time of the current day
	LOGURU_EXPORT extern bool      g_preamble_uptime; // The time since init call
	LOGURU_EXPORT extern bool      g_preamble_thread; // The logging thread
	LOGURU_EXPORT extern bool      g_preamble_file; // The file from which the log originates from
	LOGURU_EXPORT extern bool      g_preamble_verbose; // The verbosity field
	LOGURU_EXPORT extern bool      g_preamble_pipe; // The pipe symbol right before the message

	// May not throw!
	typedef void (*log_handler_t)(void* user_data, const Message& message);
	typedef void (*close_handler_t)(void* user_data);
	typedef void (*flush_handler_t)(void* user_data);

	// May throw if that's how you'd like to handle your errors.
	typedef void (*fatal_handler_t)(const Message& message);

	// Given a verbosity level, return the level's name or nullptr.
	typedef const char* (*verbosity_to_name_t)(Verbosity verbosity);

	// Given a verbosity level name, return the verbosity level or
	// Verbosity_INVALID if name is not recognized.
	typedef Verbosity (*name_to_verbosity_t)(const char* name);

	struct SignalOptions
	{
		/// Make Loguru try to do unsafe but useful things,
		/// like printing a stack trace, when catching signals.
		/// This may lead to bad things like deadlocks in certain situations.
		bool unsafe_signal_handler = true;

		/// Should Loguru catch SIGABRT ?
		bool sigabrt = true;

		/// Should Loguru catch SIGBUS ?
		bool sigbus = true;

		/// Should Loguru catch SIGFPE ?
		bool sigfpe = true;

		/// Should Loguru catch SIGILL ?
		bool sigill = true;

		/// Should Loguru catch SIGINT ?
		bool sigint = true;

		/// Should Loguru catch SIGSEGV ?
		bool sigsegv = true;

		/// Should Loguru catch SIGTERM ?
		bool sigterm = true;

		static SignalOptions none()
		{
			SignalOptions options;
			options.unsafe_signal_handler = false;
			options.sigabrt = false;
			options.sigbus = false;
			options.sigfpe = false;
			options.sigill = false;
			options.sigint = false;
			options.sigsegv = false;
			options.sigterm = false;
			return options;
		}
	};

	// Runtime options passed to loguru::init
	struct Options
	{
		// This allows you to use something else instead of "-v" via verbosity_flag.
		// Set to nullptr if you don't want Loguru to parse verbosity from the args.
		const char* verbosity_flag = "-v";

		// loguru::init will set the name of the calling thread to this.
		// If you don't want Loguru to set the name of the main thread,
		// set this to nullptr.
		// NOTE: on SOME platforms loguru::init will only overwrite the thread name
		// if a thread name has not already been set.
		// To always set a thread name, use loguru::set_thread_name instead.
		const char* main_thread_name = "main thread";

		SignalOptions signal_options;
	};

	/*  Should be called from the main thread.
		You don't *need* to call this, but if you do you get:
			* Signal handlers installed
			* Program arguments logged
			* Working dir logged
			* Optional -v verbosity flag parsed
			* Main thread name set to "main thread"
			* Explanation of the preamble (date, thread name, etc) logged

		loguru::init() will look for arguments meant for loguru and remove them.
		Arguments meant for loguru are:
			-v n   Set loguru::g_stderr_verbosity level. Examples:
				-v 3        Show verbosity level 3 and lower.
				-v 0        Only show INFO, WARNING, ERROR, FATAL (default).
				-v INFO     Only show INFO, WARNING, ERROR, FATAL (default).
				-v WARNING  Only show WARNING, ERROR, FATAL.
				-v ERROR    Only show ERROR, FATAL.
				-v FATAL    Only show FATAL.
				-v OFF      Turn off logging to stderr.

		Tip: You can set g_stderr_verbosity before calling loguru::init.
		That way you can set the default but have the user override it with the -v flag.
		Note that -v does not affect file logging (see loguru::add_file).

		You can you something other than the -v flag by setting the verbosity_flag option.
	*/
	LOGURU_EXPORT
	void init(int& argc, char* argv[], const Options& options = {});

	// Will call remove_all_callbacks(). After calling this, logging will still go to stderr.
	// You generally don't need to call this.
	LOGURU_EXPORT
	void shutdown();

	// What ~ will be replaced with, e.g. "/home/your_user_name/"
	LOGURU_EXPORT
	const char* home_dir();

	/* Returns the name of the app as given in argv[0] but without leading path.
	   That is, if argv[0] is "../foo/app" this will return "app".
	*/
	LOGURU_EXPORT
	const char* argv0_filename();

	// Returns all arguments given to loguru::init(), but escaped with a single space as separator.
	LOGURU_EXPORT
	const char* arguments();

	// Returns the path to the current working dir when loguru::init() was called.
	LOGURU_EXPORT
	const char* current_dir();

	// Returns the part of the path after the last / or \ (if any).
	LOGURU_EXPORT
	const char* filename(const char* path);

	// e.g. "foo/bar/baz.ext" will create the directories "foo/" and "foo/bar/"
	LOGURU_EXPORT
	bool create_directories(const char* file_path_const);

	// Writes date and time with millisecond precision, e.g. "20151017_161503.123"
	LOGURU_EXPORT
	void write_date_time(char* buff, unsigned long long buff_size);

	// Helper: thread-safe version strerror
	LOGURU_EXPORT
	Text errno_as_text();

	/* Given a prefix of e.g. "~/loguru/" this might return
	   "/home/your_username/loguru/app_name/20151017_161503.123.log"

	   where "app_name" is a sanitized version of argv[0].
	*/
	LOGURU_EXPORT
	void suggest_log_path(const char* prefix, char* buff, unsigned long long buff_size);

	enum FileMode { Truncate, Append };

	/*  Will log to a file at the given path.
		Any logging message with a verbosity lower or equal to
		the given verbosity will be included.
		The function will create all directories in 'path' if needed.
		If path starts with a ~, it will be replaced with loguru::home_dir()
		To stop the file logging, just call loguru::remove_callback(path) with the same path.
	*/
	LOGURU_EXPORT
	bool add_file(const char* path, FileMode mode, Verbosity verbosity);

	LOGURU_EXPORT
	// Send logs to syslog with LOG_USER facility (see next call)
	bool add_syslog(const char* app_name, Verbosity verbosity);
	LOGURU_EXPORT
	// Send logs to syslog with your own choice of facility (LOG_USER, LOG_AUTH, ...)
	// see loguru.cpp: syslog_log() for more details.
	bool add_syslog(const char* app_name, Verbosity verbosity, int facility);

	/*  Will be called right before abort().
		You can for instance use this to print custom error messages, or throw an exception.
		Feel free to call LOG:ing function from this, but not FATAL ones! */
	LOGURU_EXPORT
	void set_fatal_handler(fatal_handler_t handler);

	// Get the current fatal handler, if any. Default value is nullptr.
	LOGURU_EXPORT
	fatal_handler_t get_fatal_handler();

	/*  Will be called on each log messages with a verbosity less or equal to the given one.
		Useful for displaying messages on-screen in a game, for example.
		The given on_close is also expected to flush (if desired).
	*/
	LOGURU_EXPORT
	void add_callback(
		const char*     id,
		log_handler_t   callback,
		void*           user_data,
		Verbosity       verbosity,
		close_handler_t on_close = nullptr,
		flush_handler_t on_flush = nullptr);

	/*  Set a callback that returns custom verbosity level names. If callback
		is nullptr or returns nullptr, default log names will be used.
	*/
	LOGURU_EXPORT
	void set_verbosity_to_name_callback(verbosity_to_name_t callback);

	/*  Set a callback that returns the verbosity level matching a name. The
		callback should return Verbosity_INVALID if the name is not
		recognized.
	*/
	LOGURU_EXPORT
	void set_name_to_verbosity_callback(name_to_verbosity_t callback);

	/*  Get a custom name for a specific verbosity, if one exists, or nullptr. */
	LOGURU_EXPORT
	const char* get_verbosity_name(Verbosity verbosity);

	/*  Get the verbosity enum value from a custom 4-character level name, if one exists.
		If the name does not match a custom level name, Verbosity_INVALID is returned.
	*/
	LOGURU_EXPORT
	Verbosity get_verbosity_from_name(const char* name);

	// Returns true iff the callback was found (and removed).
	LOGURU_EXPORT
	bool remove_callback(const char* id);

	// Shut down all file logging and any other callback hooks installed.
	LOGURU_EXPORT
	void remove_all_callbacks();

	// Returns the maximum of g_stderr_verbosity and all file/custom outputs.
	LOGURU_EXPORT
	Verbosity current_verbosity_cutoff();

#if LOGURU_USE_FMTLIB
	// Internal functions
    LOGURU_EXPORT
	void vlog(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, fmt::format_args args);
    LOGURU_EXPORT
	void raw_vlog(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, fmt::format_args args);

	// Actual logging function. Use the LOG macro instead of calling this directly.
	template <typename... Args>
	LOGURU_EXPORT
	void log(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, const Args &... args) {
	    vlog(verbosity, file, line, format, fmt::make_format_args(args...));
	}

	// Log without any preamble or indentation.
	template <typename... Args>
	LOGURU_EXPORT
	void raw_log(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, const Args &... args) {
	    raw_vlog(verbosity, file, line, format, fmt::make_format_args(args...));
	}
#else // LOGURU_USE_FMTLIB?
	// Actual logging function. Use the LOG macro instead of calling this directly.
	LOGURU_EXPORT
	void log(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, ...) LOGURU_PRINTF_LIKE(4, 5);

	// Actual logging function.
	LOGURU_EXPORT
	void vlog(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, va_list) LOGURU_PRINTF_LIKE(4, 0);

	// Log without any preamble or indentation.
	LOGURU_EXPORT
	void raw_log(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, ...) LOGURU_PRINTF_LIKE(4, 5);
#endif // !LOGURU_USE_FMTLIB

	// Helper class for LOG_SCOPE_F
	class LOGURU_EXPORT LogScopeRAII
	{
	public:
		LogScopeRAII() : _file(nullptr) {} // No logging
		LogScopeRAII(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, va_list vlist) LOGURU_PRINTF_LIKE(5, 0);
		LogScopeRAII(Verbosity verbosity, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, ...) LOGURU_PRINTF_LIKE(5, 6);
		~LogScopeRAII();

		void Init(LOGURU_FORMAT_STRING_TYPE format, va_list vlist) LOGURU_PRINTF_LIKE(2, 0);

#if defined(_MSC_VER) && _MSC_VER > 1800
		// older MSVC default move ctors close the scope on move. See
		// issue #43
		LogScopeRAII(LogScopeRAII&& other)
			: _verbosity(other._verbosity)
			, _file(other._file)
			, _line(other._line)
			, _indent_stderr(other._indent_stderr)
			, _start_time_ns(other._start_time_ns)
		{
			// Make sure the tmp object's destruction doesn't close the scope:
			other._file = nullptr;

			for (unsigned int i = 0; i < LOGURU_SCOPE_TEXT_SIZE; ++i) {
				_name[i] = other._name[i];
			}
		}
#else
		LogScopeRAII(LogScopeRAII&&) = default;
#endif

	private:
		LogScopeRAII(const LogScopeRAII&) = delete;
		LogScopeRAII& operator=(const LogScopeRAII&) = delete;
		void operator=(LogScopeRAII&&) = delete;

		Verbosity   _verbosity;
		const char* _file; // Set to null if we are disabled due to verbosity
		unsigned    _line;
		bool        _indent_stderr; // Did we?
		long long   _start_time_ns;
		char        _name[LOGURU_SCOPE_TEXT_SIZE];
	};

	// Marked as 'noreturn' for the benefit of the static analyzer and optimizer.
	// stack_trace_skip is the number of extrace stack frames to skip above log_and_abort.
#if LOGURU_USE_FMTLIB
	LOGURU_EXPORT
	LOGURU_NORETURN void vlog_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, fmt::format_args);
	template <typename... Args>
	LOGURU_EXPORT
	LOGURU_NORETURN void log_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, const Args&... args) {
	    vlog_and_abort(stack_trace_skip, expr, file, line, format, fmt::make_format_args(args...));
	}
#else
	LOGURU_EXPORT
	LOGURU_NORETURN void log_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line, LOGURU_FORMAT_STRING_TYPE format, ...) LOGURU_PRINTF_LIKE(5, 6);
#endif
	LOGURU_EXPORT
	LOGURU_NORETURN void log_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line);

	// Flush output to stderr and files.
	// If g_flush_interval_ms is set to non-zero, this will be called automatically this often.
	// If not set, you do not need to call this at all.
	LOGURU_EXPORT
	void flush();

	template<class T> inline Text format_value(const T&)                    { return textprintf("N/A");     }
	template<>        inline Text format_value(const char& v)               { return textprintf(LOGURU_FMT(c),   v); }
	template<>        inline Text format_value(const int& v)                { return textprintf(LOGURU_FMT(d),   v); }
	template<>        inline Text format_value(const float& v)              { return textprintf(LOGURU_FMT(f),   v); }
	template<>        inline Text format_value(const double& v)             { return textprintf(LOGURU_FMT(f),   v); }

#if LOGURU_USE_FMTLIB
	template<>        inline Text format_value(const unsigned int& v)       { return textprintf(LOGURU_FMT(d), v); }
	template<>        inline Text format_value(const long& v)               { return textprintf(LOGURU_FMT(d), v); }
	template<>        inline Text format_value(const unsigned long& v)      { return textprintf(LOGURU_FMT(d), v); }
	template<>        inline Text format_value(const long long& v)          { return textprintf(LOGURU_FMT(d), v); }
	template<>        inline Text format_value(const unsigned long long& v) { return textprintf(LOGURU_FMT(d), v); }
#else
	template<>        inline Text format_value(const unsigned int& v)       { return textprintf(LOGURU_FMT(u),   v); }
	template<>        inline Text format_value(const long& v)               { return textprintf(LOGURU_FMT(lu),  v); }
	template<>        inline Text format_value(const unsigned long& v)      { return textprintf(LOGURU_FMT(ld),  v); }
	template<>        inline Text format_value(const long long& v)          { return textprintf(LOGURU_FMT(llu), v); }
	template<>        inline Text format_value(const unsigned long long& v) { return textprintf(LOGURU_FMT(lld), v); }
#endif

	/* Thread names can be set for the benefit of readable logs.
	   If you do not set the thread name, a hex id will be shown instead.
	   These thread names may or may not be the same as the system thread names,
	   depending on the system.
	   Try to limit the thread name to 15 characters or less. */
	LOGURU_EXPORT
	void set_thread_name(const char* name);

	/* Returns the thread name for this thread.
	   On most *nix systems this will return the system thread name (settable from both within and without Loguru).
	   On other systems it will return whatever you set in `set_thread_name()`;
	   If no thread name is set, this will return a hexadecimal thread id.
	   `length` should be the number of bytes available in the buffer.
	   17 is a good number for length.
	   `right_align_hex_id` means any hexadecimal thread id will be written to the end of buffer.
	*/
	LOGURU_EXPORT
	void get_thread_name(char* buffer, unsigned long long length, bool right_align_hex_id);

	/* Generates a readable stacktrace as a string.
	   'skip' specifies how many stack frames to skip.
	   For instance, the default skip (1) means:
	   don't include the call to loguru::stacktrace in the stack trace. */
	LOGURU_EXPORT
	Text stacktrace(int skip = 1);

	/*  Add a string to be replaced with something else in the stack output.

		For instance, instead of having a stack trace look like this:
			0x41f541 some_function(std::basic_ofstream<char, std::char_traits<char> >&)
		You can clean it up with:
			auto verbose_type_name = loguru::demangle(typeid(std::ofstream).name());
			loguru::add_stack_cleanup(verbose_type_name.c_str(); "std::ofstream");
		So the next time you will instead see:
			0x41f541 some_function(std::ofstream&)

		`replace_with_this` must be shorter than `find_this`.
	*/
	LOGURU_EXPORT
	void add_stack_cleanup(const char* find_this, const char* replace_with_this);

	// Example: demangle(typeid(std::ofstream).name()) -> "std::basic_ofstream<char, std::char_traits<char> >"
	LOGURU_EXPORT
	Text demangle(const char* name);

	// ------------------------------------------------------------------------
	/*
	Not all terminals support colors, but if they do, and g_colorlogtostderr
	is set, Loguru will write them to stderr to make errors in red, etc.

	You also have the option to manually use them, via the function below.

	Note, however, that if you do, the color codes could end up in your logfile!

	This means if you intend to use them functions you should either:
		* Use them on the stderr/stdout directly (bypass Loguru).
		* Don't add file outputs to Loguru.
		* Expect some \e[1m things in your logfile.

	Usage:
		printf("%sRed%sGreen%sBold green%sClear again\n",
			   loguru::terminal_red(), loguru::terminal_green(),
			   loguru::terminal_bold(), loguru::terminal_reset());

	If the terminal at hand does not support colors the above output
	will just not have funky \e[1m things showing.
	*/

	// Do the output terminal support colors?
	LOGURU_EXPORT
	bool terminal_has_color();

	// Colors
	LOGURU_EXPORT const char* terminal_black();
	LOGURU_EXPORT const char* terminal_red();
	LOGURU_EXPORT const char* terminal_green();
	LOGURU_EXPORT const char* terminal_yellow();
	LOGURU_EXPORT const char* terminal_blue();
	LOGURU_EXPORT const char* terminal_purple();
	LOGURU_EXPORT const char* terminal_cyan();
	LOGURU_EXPORT const char* terminal_light_gray();
	LOGURU_EXPORT const char* terminal_light_red();
	LOGURU_EXPORT const char* terminal_white();

	// Formating
	LOGURU_EXPORT const char* terminal_bold();
	LOGURU_EXPORT const char* terminal_underline();

	// You should end each line with this!
	LOGURU_EXPORT const char* terminal_reset();

	// --------------------------------------------------------------------
	// Error context related:

	struct StringStream;

	// Use this in your EcEntryBase::print_value overload.
	LOGURU_EXPORT
	void stream_print(StringStream& out_string_stream, const char* text);

	class LOGURU_EXPORT EcEntryBase
	{
	public:
		EcEntryBase(const char* file, unsigned line, const char* descr);
		~EcEntryBase();
		EcEntryBase(const EcEntryBase&) = delete;
		EcEntryBase(EcEntryBase&&) = delete;
		EcEntryBase& operator=(const EcEntryBase&) = delete;
		EcEntryBase& operator=(EcEntryBase&&) = delete;

		virtual void print_value(StringStream& out_string_stream) const = 0;

		EcEntryBase* previous() const { return _previous; }

	// private:
		const char*  _file;
		unsigned     _line;
		const char*  _descr;
		EcEntryBase* _previous;
	};

	template<typename T>
	class EcEntryData : public EcEntryBase
	{
	public:
		using Printer = Text(*)(T data);

		EcEntryData(const char* file, unsigned line, const char* descr, T data, Printer&& printer)
			: EcEntryBase(file, line, descr), _data(data), _printer(printer) {}

		virtual void print_value(StringStream& out_string_stream) const override
		{
			const auto str = _printer(_data);
			stream_print(out_string_stream, str.c_str());
		}

	private:
		T       _data;
		Printer _printer;
	};

	// template<typename Printer>
	// class EcEntryLambda : public EcEntryBase
	// {
	// public:
	// 	EcEntryLambda(const char* file, unsigned line, const char* descr, Printer&& printer)
	// 		: EcEntryBase(file, line, descr), _printer(std::move(printer)) {}

	// 	virtual void print_value(StringStream& out_string_stream) const override
	// 	{
	// 		const auto str = _printer();
	// 		stream_print(out_string_stream, str.c_str());
	// 	}

	// private:
	// 	Printer _printer;
	// };

	// template<typename Printer>
	// EcEntryLambda<Printer> make_ec_entry_lambda(const char* file, unsigned line, const char* descr, Printer&& printer)
	// {
	// 	return {file, line, descr, std::move(printer)};
	// }

	template <class T>
	struct decay_char_array { using type = T; };

	template <unsigned long long  N>
	struct decay_char_array<const char(&)[N]> { using type = const char*; };

	template <class T>
	struct make_const_ptr { using type = T; };

	template <class T>
	struct make_const_ptr<T*> { using type = const T*; };

	template <class T>
	struct make_ec_type { using type = typename make_const_ptr<typename decay_char_array<T>::type>::type; };

	/* 	A stack trace gives you the names of the function at the point of a crash.
		With ERROR_CONTEXT, you can also get the values of select local variables.
		Usage:

		void process_customers(const std::string& filename)
		{
			ERROR_CONTEXT("Processing file", filename.c_str());
			for (int customer_index : ...)
			{
				ERROR_CONTEXT("Customer index", customer_index);
				...
			}
		}

		The context is in effect during the scope of the ERROR_CONTEXT.
		Use loguru::get_error_context() to get the contents of the active error contexts.

		Example result:

		------------------------------------------------
		[ErrorContext]                main.cpp:416   Processing file:    "customers.json"
		[ErrorContext]                main.cpp:417   Customer index:     42
		------------------------------------------------

		Error contexts are printed automatically on crashes, and only on crashes.
		This makes them much faster than logging the value of a variable.
	*/
	#define ERROR_CONTEXT(descr, data)                                             \
		const loguru::EcEntryData<loguru::make_ec_type<decltype(data)>::type>      \
			LOGURU_ANONYMOUS_VARIABLE(error_context_scope_)(                       \
				__FILE__, __LINE__, descr, data,                                   \
				static_cast<loguru::EcEntryData<loguru::make_ec_type<decltype(data)>::type>::Printer>(loguru::ec_to_text) ) // For better error messages

/*
	#define ERROR_CONTEXT(descr, data)                                 \
		const auto LOGURU_ANONYMOUS_VARIABLE(error_context_scope_)(    \
			loguru::make_ec_entry_lambda(__FILE__, __LINE__, descr,    \
				[=](){ return loguru::ec_to_text(data); }))
*/

	using EcHandle = const EcEntryBase*;

	/*
		Get a light-weight handle to the error context stack on this thread.
		The handle is valid as long as the current thread has no changes to its error context stack.
		You can pass the handle to loguru::get_error_context on another thread.
		This can be very useful for when you have a parent thread spawning several working threads,
		and you want the error context of the parent thread to get printed (too) when there is an
		error on the child thread. You can accomplish this thusly:

		void foo(const char* parameter)
		{
			ERROR_CONTEXT("parameter", parameter)
			const auto parent_ec_handle = loguru::get_thread_ec_handle();

			std::thread([=]{
				loguru::set_thread_name("child thread");
				ERROR_CONTEXT("parent context", parent_ec_handle);
				dangerous_code();
			}.join();
		}

	*/
	LOGURU_EXPORT
	EcHandle get_thread_ec_handle();

	// Get a string describing the current stack of error context. Empty string if there is none.
	LOGURU_EXPORT
	Text get_error_context();

	// Get a string describing the error context of the given thread handle.
	LOGURU_EXPORT
	Text get_error_context_for(EcHandle ec_handle);

	// ------------------------------------------------------------------------

	LOGURU_EXPORT Text ec_to_text(const char* data);
	LOGURU_EXPORT Text ec_to_text(char data);
	LOGURU_EXPORT Text ec_to_text(int data);
	LOGURU_EXPORT Text ec_to_text(unsigned int data);
	LOGURU_EXPORT Text ec_to_text(long data);
	LOGURU_EXPORT Text ec_to_text(unsigned long data);
	LOGURU_EXPORT Text ec_to_text(long long data);
	LOGURU_EXPORT Text ec_to_text(unsigned long long data);
	LOGURU_EXPORT Text ec_to_text(float data);
	LOGURU_EXPORT Text ec_to_text(double data);
	LOGURU_EXPORT Text ec_to_text(long double data);
	LOGURU_EXPORT Text ec_to_text(EcHandle);

	/*
	You can add ERROR_CONTEXT support for your own types by overloading ec_to_text. Here's how:

	some.hpp:
		namespace loguru {
			Text ec_to_text(MySmallType data)
			Text ec_to_text(const MyBigType* data)
		} // namespace loguru

	some.cpp:
		namespace loguru {
			Text ec_to_text(MySmallType small_value)
			{
				// Called only when needed, i.e. on a crash.
				std::string str = small_value.as_string(); // Format 'small_value' here somehow.
				return Text{STRDUP(str.c_str())};
			}

			Text ec_to_text(const MyBigType* big_value)
			{
				// Called only when needed, i.e. on a crash.
				std::string str = big_value->as_string(); // Format 'big_value' here somehow.
				return Text{STRDUP(str.c_str())};
			}
		} // namespace loguru

	Any file that include some.hpp:
		void foo(MySmallType small, const MyBigType& big)
		{
			ERROR_CONTEXT("Small", small); // Copy ´small` by value.
			ERROR_CONTEXT("Big",   &big);  // `big` should not change during this scope!
			....
		}
	*/
} // namespace loguru

LOGURU_ANONYMOUS_NAMESPACE_END

// --------------------------------------------------------------------
// Logging macros

// LOG_F(2, "Only logged if verbosity is 2 or higher: %d", some_number);
#define VLOG_F(verbosity, ...)                                                                     \
	((verbosity) > loguru::current_verbosity_cutoff()) ? (void)0                                   \
									  : loguru::log(verbosity, __FILE__, __LINE__, __VA_ARGS__)

// LOG_F(INFO, "Foo: %d", some_number);
#define LOG_F(verbosity_name, ...) VLOG_F(loguru::Verbosity_ ## verbosity_name, __VA_ARGS__)

#define VLOG_IF_F(verbosity, cond, ...)                                                            \
	((verbosity) > loguru::current_verbosity_cutoff() || (cond) == false)                          \
		? (void)0                                                                                  \
		: loguru::log(verbosity, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_IF_F(verbosity_name, cond, ...)                                                        \
	VLOG_IF_F(loguru::Verbosity_ ## verbosity_name, cond, __VA_ARGS__)

#define VLOG_SCOPE_F(verbosity, ...)                                                               \
	loguru::LogScopeRAII LOGURU_ANONYMOUS_VARIABLE(error_context_RAII_) =                          \
	((verbosity) > loguru::current_verbosity_cutoff()) ? loguru::LogScopeRAII() :                  \
	loguru::LogScopeRAII(verbosity, __FILE__, __LINE__, __VA_ARGS__)

// Raw logging - no preamble, no indentation. Slightly faster than full logging.
#define RAW_VLOG_F(verbosity, ...)                                                                 \
	((verbosity) > loguru::current_verbosity_cutoff()) ? (void)0                                   \
									  : loguru::raw_log(verbosity, __FILE__, __LINE__, __VA_ARGS__)

#define RAW_LOG_F(verbosity_name, ...) RAW_VLOG_F(loguru::Verbosity_ ## verbosity_name, __VA_ARGS__)

// Use to book-end a scope. Affects logging on all threads.
#define LOG_SCOPE_F(verbosity_name, ...)                                                           \
	VLOG_SCOPE_F(loguru::Verbosity_ ## verbosity_name, __VA_ARGS__)

#define LOG_SCOPE_FUNCTION(verbosity_name) LOG_SCOPE_F(verbosity_name, __func__)

// -----------------------------------------------
// ABORT_F macro. Usage:  ABORT_F("Cause of error: %s", error_str);

// Message is optional
#define ABORT_F(...) loguru::log_and_abort(0, "ABORT: ", __FILE__, __LINE__, __VA_ARGS__)

// --------------------------------------------------------------------
// CHECK_F macros:

#define CHECK_WITH_INFO_F(test, info, ...)                                                         \
	LOGURU_PREDICT_TRUE((test) == true) ? (void)0 : loguru::log_and_abort(0, "CHECK FAILED:  " info "  ", __FILE__,      \
													   __LINE__, ##__VA_ARGS__)

/* Checked at runtime too. Will print error, then call fatal_handler (if any), then 'abort'.
   Note that the test must be boolean.
   CHECK_F(ptr); will not compile, but CHECK_F(ptr != nullptr); will. */
#define CHECK_F(test, ...) CHECK_WITH_INFO_F(test, #test, ##__VA_ARGS__)

#define CHECK_NOTNULL_F(x, ...) CHECK_WITH_INFO_F((x) != nullptr, #x " != nullptr", ##__VA_ARGS__)

#define CHECK_OP_F(expr_left, expr_right, op, ...)                                                 \
	do                                                                                             \
	{                                                                                              \
		auto val_left = expr_left;                                                                 \
		auto val_right = expr_right;                                                               \
		if (! LOGURU_PREDICT_TRUE(val_left op val_right))                                          \
		{                                                                                          \
			auto str_left = loguru::format_value(val_left);                                        \
			auto str_right = loguru::format_value(val_right);                                      \
			auto fail_info = loguru::textprintf("CHECK FAILED:  " LOGURU_FMT(s) " " LOGURU_FMT(s) " " LOGURU_FMT(s) "  (" LOGURU_FMT(s) " " LOGURU_FMT(s) " " LOGURU_FMT(s) ")  ",           \
				#expr_left, #op, #expr_right, str_left.c_str(), #op, str_right.c_str());           \
			auto user_msg = loguru::textprintf(__VA_ARGS__);                                       \
			loguru::log_and_abort(0, fail_info.c_str(), __FILE__, __LINE__,                        \
			                      LOGURU_FMT(s), user_msg.c_str());                                         \
		}                                                                                          \
	} while (false)

#ifndef LOGURU_DEBUG_LOGGING
	#ifndef NDEBUG
		#define LOGURU_DEBUG_LOGGING 1
	#else
		#define LOGURU_DEBUG_LOGGING 0
	#endif
#endif

#if LOGURU_DEBUG_LOGGING
	// Debug logging enabled:
	#define DLOG_F(verbosity_name, ...)     LOG_F(verbosity_name, __VA_ARGS__)
	#define DVLOG_F(verbosity, ...)         VLOG_F(verbosity, __VA_ARGS__)
	#define DLOG_IF_F(verbosity_name, ...)  LOG_IF_F(verbosity_name, __VA_ARGS__)
	#define DVLOG_IF_F(verbosity, ...)      VLOG_IF_F(verbosity, __VA_ARGS__)
	#define DRAW_LOG_F(verbosity_name, ...) RAW_LOG_F(verbosity_name, __VA_ARGS__)
	#define DRAW_VLOG_F(verbosity, ...)     RAW_VLOG_F(verbosity, __VA_ARGS__)
#else
	// Debug logging disabled:
	#define DLOG_F(verbosity_name, ...)
	#define DVLOG_F(verbosity, ...)
	#define DLOG_IF_F(verbosity_name, ...)
	#define DVLOG_IF_F(verbosity, ...)
	#define DRAW_LOG_F(verbosity_name, ...)
	#define DRAW_VLOG_F(verbosity, ...)
#endif

#define CHECK_EQ_F(a, b, ...) CHECK_OP_F(a, b, ==, ##__VA_ARGS__)
#define CHECK_NE_F(a, b, ...) CHECK_OP_F(a, b, !=, ##__VA_ARGS__)
#define CHECK_LT_F(a, b, ...) CHECK_OP_F(a, b, < , ##__VA_ARGS__)
#define CHECK_GT_F(a, b, ...) CHECK_OP_F(a, b, > , ##__VA_ARGS__)
#define CHECK_LE_F(a, b, ...) CHECK_OP_F(a, b, <=, ##__VA_ARGS__)
#define CHECK_GE_F(a, b, ...) CHECK_OP_F(a, b, >=, ##__VA_ARGS__)

#ifndef LOGURU_DEBUG_CHECKS
	#ifndef NDEBUG
		#define LOGURU_DEBUG_CHECKS 1
	#else
		#define LOGURU_DEBUG_CHECKS 0
	#endif
#endif

#if LOGURU_DEBUG_CHECKS
	// Debug checks enabled:
	#define DCHECK_F(test, ...)             CHECK_F(test, ##__VA_ARGS__)
	#define DCHECK_NOTNULL_F(x, ...)        CHECK_NOTNULL_F(x, ##__VA_ARGS__)
	#define DCHECK_EQ_F(a, b, ...)          CHECK_EQ_F(a, b, ##__VA_ARGS__)
	#define DCHECK_NE_F(a, b, ...)          CHECK_NE_F(a, b, ##__VA_ARGS__)
	#define DCHECK_LT_F(a, b, ...)          CHECK_LT_F(a, b, ##__VA_ARGS__)
	#define DCHECK_LE_F(a, b, ...)          CHECK_LE_F(a, b, ##__VA_ARGS__)
	#define DCHECK_GT_F(a, b, ...)          CHECK_GT_F(a, b, ##__VA_ARGS__)
	#define DCHECK_GE_F(a, b, ...)          CHECK_GE_F(a, b, ##__VA_ARGS__)
#else
	// Debug checks disabled:
	#define DCHECK_F(test, ...)
	#define DCHECK_NOTNULL_F(x, ...)
	#define DCHECK_EQ_F(a, b, ...)
	#define DCHECK_NE_F(a, b, ...)
	#define DCHECK_LT_F(a, b, ...)
	#define DCHECK_LE_F(a, b, ...)
	#define DCHECK_GT_F(a, b, ...)
	#define DCHECK_GE_F(a, b, ...)
#endif // NDEBUG


#if LOGURU_REDEFINE_ASSERT
	#undef assert
	#ifndef NDEBUG
		// Debug:
		#define assert(test) CHECK_WITH_INFO_F(!!(test), #test) // HACK
	#else
		#define assert(test)
	#endif
#endif // LOGURU_REDEFINE_ASSERT

#endif // LOGURU_HAS_DECLARED_FORMAT_HEADER

// ----------------------------------------------------------------------------
// .dP"Y8 888888 88""Yb 888888    db    8b    d8 .dP"Y8
// `Ybo."   88   88__dP 88__     dPYb   88b  d88 `Ybo."
// o.`Y8b   88   88"Yb  88""    dP__Yb  88YbdP88 o.`Y8b
// 8bodP'   88   88  Yb 888888 dP""""Yb 88 YY 88 8bodP'

#if LOGURU_WITH_STREAMS
#ifndef LOGURU_HAS_DECLARED_STREAMS_HEADER
#define LOGURU_HAS_DECLARED_STREAMS_HEADER

/* This file extends loguru to enable std::stream-style logging, a la Glog.
   It's an optional feature behind the LOGURU_WITH_STREAMS settings
   because including it everywhere will slow down compilation times.
*/

#include <cstdarg>
#include <sstream> // Adds about 38 kLoC on clang.
#include <string>

LOGURU_ANONYMOUS_NAMESPACE_BEGIN

namespace loguru
{
	// Like sprintf, but returns the formated text.
	LOGURU_EXPORT
	std::string strprintf(LOGURU_FORMAT_STRING_TYPE format, ...) LOGURU_PRINTF_LIKE(1, 2);

	// Like vsprintf, but returns the formated text.
	LOGURU_EXPORT
	std::string vstrprintf(LOGURU_FORMAT_STRING_TYPE format, va_list) LOGURU_PRINTF_LIKE(1, 0);

	class LOGURU_EXPORT StreamLogger
	{
	public:
		StreamLogger(Verbosity verbosity, const char* file, unsigned line) : _verbosity(verbosity), _file(file), _line(line) {}
		~StreamLogger() noexcept(false);

		template<typename T>
		StreamLogger& operator<<(const T& t)
		{
			_ss << t;
			return *this;
		}

		// std::endl and other iomanip:s.
		StreamLogger& operator<<(std::ostream&(*f)(std::ostream&))
		{
			f(_ss);
			return *this;
		}

	private:
		Verbosity   _verbosity;
		const char* _file;
		unsigned    _line;
		std::ostringstream _ss;
	};

	class LOGURU_EXPORT AbortLogger
	{
	public:
		AbortLogger(const char* expr, const char* file, unsigned line) : _expr(expr), _file(file), _line(line) { }
		LOGURU_NORETURN ~AbortLogger() noexcept(false);

		template<typename T>
		AbortLogger& operator<<(const T& t)
		{
			_ss << t;
			return *this;
		}

		// std::endl and other iomanip:s.
		AbortLogger& operator<<(std::ostream&(*f)(std::ostream&))
		{
			f(_ss);
			return *this;
		}

	private:
		const char*        _expr;
		const char*        _file;
		unsigned           _line;
		std::ostringstream _ss;
	};

	class LOGURU_EXPORT Voidify
	{
	public:
		Voidify() {}
		// This has to be an operator with a precedence lower than << but higher than ?:
		void operator&(const StreamLogger&) { }
		void operator&(const AbortLogger&)  { }
	};

	/*  Helper functions for CHECK_OP_S macro.
		GLOG trick: The (int, int) specialization works around the issue that the compiler
		will not instantiate the template version of the function on values of unnamed enum type. */
	#define DEFINE_CHECK_OP_IMPL(name, op)                                                             \
		template <typename T1, typename T2>                                                            \
		inline std::string* name(const char* expr, const T1& v1, const char* op_str, const T2& v2)     \
		{                                                                                              \
			if (LOGURU_PREDICT_TRUE(v1 op v2)) { return NULL; }                                        \
			std::ostringstream ss;                                                                     \
			ss << "CHECK FAILED:  " << expr << "  (" << v1 << " " << op_str << " " << v2 << ")  ";     \
			return new std::string(ss.str());                                                          \
		}                                                                                              \
		inline std::string* name(const char* expr, int v1, const char* op_str, int v2)                 \
		{                                                                                              \
			return name<int, int>(expr, v1, op_str, v2);                                               \
		}

	DEFINE_CHECK_OP_IMPL(check_EQ_impl, ==)
	DEFINE_CHECK_OP_IMPL(check_NE_impl, !=)
	DEFINE_CHECK_OP_IMPL(check_LE_impl, <=)
	DEFINE_CHECK_OP_IMPL(check_LT_impl, < )
	DEFINE_CHECK_OP_IMPL(check_GE_impl, >=)
	DEFINE_CHECK_OP_IMPL(check_GT_impl, > )
	#undef DEFINE_CHECK_OP_IMPL

	/*  GLOG trick: Function is overloaded for integral types to allow static const integrals
		declared in classes and not defined to be used as arguments to CHECK* macros. */
	template <class T>
	inline const T&           referenceable_value(const T&           t) { return t; }
	inline char               referenceable_value(char               t) { return t; }
	inline unsigned char      referenceable_value(unsigned char      t) { return t; }
	inline signed char        referenceable_value(signed char        t) { return t; }
	inline short              referenceable_value(short              t) { return t; }
	inline unsigned short     referenceable_value(unsigned short     t) { return t; }
	inline int                referenceable_value(int                t) { return t; }
	inline unsigned int       referenceable_value(unsigned int       t) { return t; }
	inline long               referenceable_value(long               t) { return t; }
	inline unsigned long      referenceable_value(unsigned long      t) { return t; }
	inline long long          referenceable_value(long long          t) { return t; }
	inline unsigned long long referenceable_value(unsigned long long t) { return t; }
} // namespace loguru

LOGURU_ANONYMOUS_NAMESPACE_END

// -----------------------------------------------
// Logging macros:

// usage:  LOG_STREAM(INFO) << "Foo " << std::setprecision(10) << some_value;
#define VLOG_IF_S(verbosity, cond)                                                                 \
	((verbosity) > loguru::current_verbosity_cutoff() || (cond) == false)                          \
		? (void)0                                                                                  \
		: loguru::Voidify() & loguru::StreamLogger(verbosity, __FILE__, __LINE__)
#define LOG_IF_S(verbosity_name, cond) VLOG_IF_S(loguru::Verbosity_ ## verbosity_name, cond)
#define VLOG_S(verbosity)              VLOG_IF_S(verbosity, true)
#define LOG_S(verbosity_name)          VLOG_S(loguru::Verbosity_ ## verbosity_name)

// -----------------------------------------------
// ABORT_S macro. Usage:  ABORT_S() << "Causo of error: " << details;

#define ABORT_S() loguru::Voidify() & loguru::AbortLogger("ABORT: ", __FILE__, __LINE__)

// -----------------------------------------------
// CHECK_S macros:

#define CHECK_WITH_INFO_S(cond, info)                                                              \
	LOGURU_PREDICT_TRUE((cond) == true)                                                            \
		? (void)0                                                                                  \
		: loguru::Voidify() & loguru::AbortLogger("CHECK FAILED:  " info "  ", __FILE__, __LINE__)

#define CHECK_S(cond) CHECK_WITH_INFO_S(cond, #cond)
#define CHECK_NOTNULL_S(x) CHECK_WITH_INFO_S((x) != nullptr, #x " != nullptr")

#define CHECK_OP_S(function_name, expr1, op, expr2)                                                \
	while (auto error_string = loguru::function_name(#expr1 " " #op " " #expr2,                    \
													 loguru::referenceable_value(expr1), #op,      \
													 loguru::referenceable_value(expr2)))          \
		loguru::AbortLogger(error_string->c_str(), __FILE__, __LINE__)

#define CHECK_EQ_S(expr1, expr2) CHECK_OP_S(check_EQ_impl, expr1, ==, expr2)
#define CHECK_NE_S(expr1, expr2) CHECK_OP_S(check_NE_impl, expr1, !=, expr2)
#define CHECK_LE_S(expr1, expr2) CHECK_OP_S(check_LE_impl, expr1, <=, expr2)
#define CHECK_LT_S(expr1, expr2) CHECK_OP_S(check_LT_impl, expr1, < , expr2)
#define CHECK_GE_S(expr1, expr2) CHECK_OP_S(check_GE_impl, expr1, >=, expr2)
#define CHECK_GT_S(expr1, expr2) CHECK_OP_S(check_GT_impl, expr1, > , expr2)

#if LOGURU_DEBUG_LOGGING
	// Debug logging enabled:
	#define DVLOG_IF_S(verbosity, cond)     VLOG_IF_S(verbosity, cond)
	#define DLOG_IF_S(verbosity_name, cond) LOG_IF_S(verbosity_name, cond)
	#define DVLOG_S(verbosity)              VLOG_S(verbosity)
	#define DLOG_S(verbosity_name)          LOG_S(verbosity_name)
#else
	// Debug logging disabled:
	#define DVLOG_IF_S(verbosity, cond)                                                     \
		(true || (verbosity) > loguru::current_verbosity_cutoff() || (cond) == false)       \
			? (void)0                                                                       \
			: loguru::Voidify() & loguru::StreamLogger(verbosity, __FILE__, __LINE__)

	#define DLOG_IF_S(verbosity_name, cond) DVLOG_IF_S(loguru::Verbosity_ ## verbosity_name, cond)
	#define DVLOG_S(verbosity)              DVLOG_IF_S(verbosity, true)
	#define DLOG_S(verbosity_name)          DVLOG_S(loguru::Verbosity_ ## verbosity_name)
#endif

#if LOGURU_DEBUG_CHECKS
	// Debug checks enabled:
	#define DCHECK_S(cond)                  CHECK_S(cond)
	#define DCHECK_NOTNULL_S(x)             CHECK_NOTNULL_S(x)
	#define DCHECK_EQ_S(a, b)               CHECK_EQ_S(a, b)
	#define DCHECK_NE_S(a, b)               CHECK_NE_S(a, b)
	#define DCHECK_LT_S(a, b)               CHECK_LT_S(a, b)
	#define DCHECK_LE_S(a, b)               CHECK_LE_S(a, b)
	#define DCHECK_GT_S(a, b)               CHECK_GT_S(a, b)
	#define DCHECK_GE_S(a, b)               CHECK_GE_S(a, b)
#else
// Debug checks disabled:
	#define DCHECK_S(cond)                  CHECK_S(true || (cond))
	#define DCHECK_NOTNULL_S(x)             CHECK_S(true || (x) != nullptr)
	#define DCHECK_EQ_S(a, b)               CHECK_S(true || (a) == (b))
	#define DCHECK_NE_S(a, b)               CHECK_S(true || (a) != (b))
	#define DCHECK_LT_S(a, b)               CHECK_S(true || (a) <  (b))
	#define DCHECK_LE_S(a, b)               CHECK_S(true || (a) <= (b))
	#define DCHECK_GT_S(a, b)               CHECK_S(true || (a) >  (b))
	#define DCHECK_GE_S(a, b)               CHECK_S(true || (a) >= (b))
#endif

#if LOGURU_REPLACE_GLOG
	#undef LOG
	#undef VLOG
	#undef LOG_IF
	#undef VLOG_IF
	#undef CHECK
	#undef CHECK_NOTNULL
	#undef CHECK_EQ
	#undef CHECK_NE
	#undef CHECK_LT
	#undef CHECK_LE
	#undef CHECK_GT
	#undef CHECK_GE
	#undef DLOG
	#undef DVLOG
	#undef DLOG_IF
	#undef DVLOG_IF
	#undef DCHECK
	#undef DCHECK_NOTNULL
	#undef DCHECK_EQ
	#undef DCHECK_NE
	#undef DCHECK_LT
	#undef DCHECK_LE
	#undef DCHECK_GT
	#undef DCHECK_GE
	#undef VLOG_IS_ON

	#define LOG            LOG_S
	#define VLOG           VLOG_S
	#define LOG_IF         LOG_IF_S
	#define VLOG_IF        VLOG_IF_S
	#define CHECK(cond)    CHECK_S(!!(cond))
	#define CHECK_NOTNULL  CHECK_NOTNULL_S
	#define CHECK_EQ       CHECK_EQ_S
	#define CHECK_NE       CHECK_NE_S
	#define CHECK_LT       CHECK_LT_S
	#define CHECK_LE       CHECK_LE_S
	#define CHECK_GT       CHECK_GT_S
	#define CHECK_GE       CHECK_GE_S
	#define DLOG           DLOG_S
	#define DVLOG          DVLOG_S
	#define DLOG_IF        DLOG_IF_S
	#define DVLOG_IF       DVLOG_IF_S
	#define DCHECK         DCHECK_S
	#define DCHECK_NOTNULL DCHECK_NOTNULL_S
	#define DCHECK_EQ      DCHECK_EQ_S
	#define DCHECK_NE      DCHECK_NE_S
	#define DCHECK_LT      DCHECK_LT_S
	#define DCHECK_LE      DCHECK_LE_S
	#define DCHECK_GT      DCHECK_GT_S
	#define DCHECK_GE      DCHECK_GE_S
	#define VLOG_IS_ON(verbosity) ((verbosity) <= loguru::current_verbosity_cutoff())

#endif // LOGURU_REPLACE_GLOG

#endif // LOGURU_WITH_STREAMS

#endif // LOGURU_HAS_DECLARED_STREAMS_HEADER




#if defined(__GNUC__) || defined(__clang__)
// Disable all warnings from gcc/clang:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"

#pragma GCC diagnostic ignored "-Wc++98-compat"
#pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4365) // conversion from 'X' to 'Y', signed/unsigned mismatch
#endif


#ifndef LOGURU_HAS_BEEN_IMPLEMENTED
#define LOGURU_HAS_BEEN_IMPLEMENTED

#define LOGURU_PREAMBLE_WIDTH (53 + LOGURU_THREADNAME_WIDTH + LOGURU_FILENAME_WIDTH)

#undef min
#undef max

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#if LOGURU_SYSLOG
#include <syslog.h>
#else
#define LOG_USER 0
#endif

#ifdef _WIN32
	#include <direct.h>

	#define localtime_r(a, b) localtime_s(b, a) // No localtime_r with MSVC, but arguments are swapped for localtime_s
#else
	#include <signal.h>
	#include <sys/stat.h> // mkdir
	#include <unistd.h>   // STDERR_FILENO
#endif

#ifdef __linux__
	#include <linux/limits.h> // PATH_MAX
#elif !defined(_WIN32)
	#include <limits.h> // PATH_MAX
#endif

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

#ifdef __APPLE__
	#include "TargetConditionals.h"
#endif

// TODO: use defined(_POSIX_VERSION) for some of these things?

#if defined(_WIN32) || defined(__CYGWIN__)
	#define LOGURU_PTHREADS    0
	#define LOGURU_WINTHREADS  1
	#ifndef LOGURU_STACKTRACES
		#define LOGURU_STACKTRACES 0
	#endif
#else
	#define LOGURU_PTHREADS    1
	#define LOGURU_WINTHREADS  0
	#ifdef __GLIBC__
		#ifndef LOGURU_STACKTRACES
			#define LOGURU_STACKTRACES 1
		#endif
	#else
		#ifndef LOGURU_STACKTRACES
			#define LOGURU_STACKTRACES 0
		#endif
	#endif
#endif

#if LOGURU_STACKTRACES
	#include <cxxabi.h>    // for __cxa_demangle
	#include <dlfcn.h>     // for dladdr
	#include <execinfo.h>  // for backtrace
#endif // LOGURU_STACKTRACES

#if LOGURU_PTHREADS
	#include <pthread.h>
	#if defined(__FreeBSD__)
		#include <pthread_np.h>
		#include <sys/thr.h>
	#elif defined(__OpenBSD__)
		#include <pthread_np.h>
	#endif

	#ifdef __linux__
		/* On Linux, the default thread name is the same as the name of the binary.
		   Additionally, all new threads inherit the name of the thread it got forked from.
		   For this reason, Loguru use the pthread Thread Local Storage
		   for storing thread names on Linux. */
		#ifndef LOGURU_PTLS_NAMES
			#define LOGURU_PTLS_NAMES 1
		#endif
	#endif
#endif

#if LOGURU_WINTHREADS
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0502
	#endif
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#endif

#ifndef LOGURU_PTLS_NAMES
   #define LOGURU_PTLS_NAMES 0
#endif

LOGURU_ANONYMOUS_NAMESPACE_BEGIN

namespace loguru
{
	using namespace std::chrono;

#if LOGURU_WITH_FILEABS
	struct FileAbs
	{
		char path[PATH_MAX];
		char mode_str[4];
		Verbosity verbosity;
		struct stat st;
		FILE* fp;
		bool is_reopening = false; // to prevent recursive call in file_reopen.
		decltype(steady_clock::now()) last_check_time = steady_clock::now();
	};
#else
	typedef FILE* FileAbs;
#endif

	struct Callback
	{
		std::string     id;
		log_handler_t   callback;
		void*           user_data;
		Verbosity       verbosity; // Does not change!
		close_handler_t close;
		flush_handler_t flush;
		unsigned        indentation;
	};

	using CallbackVec = std::vector<Callback>;

	using StringPair     = std::pair<std::string, std::string>;
	using StringPairList = std::vector<StringPair>;

	const auto s_start_time = steady_clock::now();

	Verbosity g_stderr_verbosity  = Verbosity_0;
	bool      g_colorlogtostderr  = true;
	unsigned  g_flush_interval_ms = 0;
	bool      g_preamble_header   = true;
	bool      g_preamble          = true;

	Verbosity g_internal_verbosity = Verbosity_0;

	// Preamble details
	bool      g_preamble_date     = true;
	bool      g_preamble_time     = true;
	bool      g_preamble_uptime   = true;
	bool      g_preamble_thread   = true;
	bool      g_preamble_file     = true;
	bool      g_preamble_verbose  = true;
	bool      g_preamble_pipe     = true;

	static std::recursive_mutex  s_mutex;
	static Verbosity             s_max_out_verbosity = Verbosity_OFF;
	static std::string           s_argv0_filename;
	static std::string           s_arguments;
	static char                  s_current_dir[PATH_MAX];
	static CallbackVec           s_callbacks;
	static fatal_handler_t       s_fatal_handler   = nullptr;
	static verbosity_to_name_t   s_verbosity_to_name_callback = nullptr;
	static name_to_verbosity_t   s_name_to_verbosity_callback = nullptr;
	static StringPairList        s_user_stack_cleanups;
	static bool                  s_strip_file_path = true;
	static std::atomic<unsigned> s_stderr_indentation { 0 };

	// For periodic flushing:
	static std::thread* s_flush_thread   = nullptr;
	static bool         s_needs_flushing = false;

	static SignalOptions s_signal_options = SignalOptions::none();

	static const bool s_terminal_has_color = [](){
		#ifdef _WIN32
			#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
			#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
			#endif

			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hOut != INVALID_HANDLE_VALUE) {
				DWORD dwMode = 0;
				GetConsoleMode(hOut, &dwMode);
				dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				return SetConsoleMode(hOut, dwMode) != 0;
			}
			return false;
		#else
			if (!isatty(STDERR_FILENO)) {
				return false;
			}
			if (const char* term = getenv("TERM")) {
				return 0 == strcmp(term, "cygwin")
					|| 0 == strcmp(term, "linux")
					|| 0 == strcmp(term, "rxvt-unicode-256color")
					|| 0 == strcmp(term, "screen")
					|| 0 == strcmp(term, "screen-256color")
					|| 0 == strcmp(term, "screen.xterm-256color")
					|| 0 == strcmp(term, "tmux-256color")
					|| 0 == strcmp(term, "xterm")
					|| 0 == strcmp(term, "xterm-256color")
					|| 0 == strcmp(term, "xterm-termite")
					|| 0 == strcmp(term, "xterm-color");
			} else {
				return false;
			}
		#endif
	}();

	static void print_preamble_header(char* out_buff, size_t out_buff_size);

	// ------------------------------------------------------------------------------
	// Colors

	bool terminal_has_color() { return s_terminal_has_color; }

	// Colors

#ifdef _WIN32
#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else
#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

	const char* terminal_black()      { return s_terminal_has_color ? VTSEQ(30) : ""; }
	const char* terminal_red()        { return s_terminal_has_color ? VTSEQ(31) : ""; }
	const char* terminal_green()      { return s_terminal_has_color ? VTSEQ(32) : ""; }
	const char* terminal_yellow()     { return s_terminal_has_color ? VTSEQ(33) : ""; }
	const char* terminal_blue()       { return s_terminal_has_color ? VTSEQ(34) : ""; }
	const char* terminal_purple()     { return s_terminal_has_color ? VTSEQ(35) : ""; }
	const char* terminal_cyan()       { return s_terminal_has_color ? VTSEQ(36) : ""; }
	const char* terminal_light_gray() { return s_terminal_has_color ? VTSEQ(37) : ""; }
	const char* terminal_white()      { return s_terminal_has_color ? VTSEQ(37) : ""; }
	const char* terminal_light_red()  { return s_terminal_has_color ? VTSEQ(91) : ""; }
	const char* terminal_dim()        { return s_terminal_has_color ? VTSEQ(2)  : ""; }

	// Formating
	const char* terminal_bold()       { return s_terminal_has_color ? VTSEQ(1) : ""; }
	const char* terminal_underline()  { return s_terminal_has_color ? VTSEQ(4) : ""; }

	// You should end each line with this!
	const char* terminal_reset()      { return s_terminal_has_color ? VTSEQ(0) : ""; }

	// ------------------------------------------------------------------------------
#if LOGURU_WITH_FILEABS
	void file_reopen(void* user_data);
	inline FILE* to_file(void* user_data) { return reinterpret_cast<FileAbs*>(user_data)->fp; }
#else
	inline FILE* to_file(void* user_data) { return reinterpret_cast<FILE*>(user_data); }
#endif

	void file_log(void* user_data, const Message& message)
	{
#if LOGURU_WITH_FILEABS
		FileAbs* file_abs = reinterpret_cast<FileAbs*>(user_data);
		if (file_abs->is_reopening) {
			return;
		}
		// It is better checking file change every minute/hour/day,
		// instead of doing this every time we log.
		// Here check_interval is set to zero to enable checking every time;
		const auto check_interval = seconds(0);
		if (duration_cast<seconds>(steady_clock::now() - file_abs->last_check_time) > check_interval) {
			file_abs->last_check_time = steady_clock::now();
			file_reopen(user_data);
		}
		FILE* file = to_file(user_data);
		if (!file) {
			return;
		}
#else
		FILE* file = to_file(user_data);
#endif
		fprintf(file, "%s%s%s%s\n",
			message.preamble, message.indentation, message.prefix, message.message);
		if (g_flush_interval_ms == 0) {
			fflush(file);
		}
	}

	void file_close(void* user_data)
	{
		FILE* file = to_file(user_data);
		if (file) {
			fclose(file);
		}
#if LOGURU_WITH_FILEABS
		delete reinterpret_cast<FileAbs*>(user_data);
#endif
	}

	void file_flush(void* user_data)
	{
		FILE* file = to_file(user_data);
		fflush(file);
	}

#if LOGURU_WITH_FILEABS
	void file_reopen(void* user_data)
	{
		FileAbs * file_abs = reinterpret_cast<FileAbs*>(user_data);
		struct stat st;
		int ret;
		if (!file_abs->fp || (ret = stat(file_abs->path, &st)) == -1 || (st.st_ino != file_abs->st.st_ino)) {
			file_abs->is_reopening = true;
			if (file_abs->fp) {
				fclose(file_abs->fp);
			}
			if (!file_abs->fp) {
				VLOG_F(g_internal_verbosity, "Reopening file '" LOGURU_FMT(s) "' due to previous error", file_abs->path);
			}
			else if (ret < 0) {
				const auto why = errno_as_text();
				VLOG_F(g_internal_verbosity, "Reopening file '" LOGURU_FMT(s) "' due to '" LOGURU_FMT(s) "'", file_abs->path, why.c_str());
			} else {
				VLOG_F(g_internal_verbosity, "Reopening file '" LOGURU_FMT(s) "' due to file changed", file_abs->path);
			}
			// try reopen current file.
			if (!create_directories(file_abs->path)) {
				LOG_F(ERROR, "Failed to create directories to '" LOGURU_FMT(s) "'", file_abs->path);
			}
			file_abs->fp = fopen(file_abs->path, file_abs->mode_str);
			if (!file_abs->fp) {
				LOG_F(ERROR, "Failed to open '" LOGURU_FMT(s) "'", file_abs->path);
			} else {
				stat(file_abs->path, &file_abs->st);
			}
			file_abs->is_reopening = false;
		}
	}
#endif
	// ------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------
#if LOGURU_SYSLOG
	void syslog_log(void* /*user_data*/, const Message& message)
	{
		/*
			Level 0: Is reserved for kernel panic type situations.
			Level 1: Is for Major resource failure.
			Level 2->7 Application level failures
		*/
		int level;
		if (message.verbosity < Verbosity_FATAL) {
			level = 1; // System Alert
		} else {
			switch(message.verbosity) {
				case Verbosity_FATAL:   level = 2; break;	// System Critical
				case Verbosity_ERROR:   level = 3; break;	// System Error
				case Verbosity_WARNING: level = 4; break;	// System Warning
				case Verbosity_INFO:    level = 5; break;	// System Notice
				case Verbosity_1:       level = 6; break;	// System Info
				default:                level = 7; break;	// System Debug
			}
		}

		// Note: We don't add the time info.
		// This is done automatically by the syslog deamon.
		// Otherwise log all information that the file log does.
		syslog(level, "%s%s%s", message.indentation, message.prefix, message.message);
	}

	void syslog_close(void* /*user_data*/)
	{
		closelog();
	}

	void syslog_flush(void* /*user_data*/)
	{}
#endif
// ------------------------------------------------------------------------------
	// Helpers:

	Text::~Text() { free(_str); }

#if LOGURU_USE_FMTLIB
	Text vtextprintf(const char* format, fmt::format_args args)
	{
		return Text(STRDUP(fmt::vformat(format, args).c_str()));
	}
#else
	LOGURU_PRINTF_LIKE(1, 0)
	static Text vtextprintf(const char* format, va_list vlist)
	{
#ifdef _WIN32
		int bytes_needed = _vscprintf(format, vlist);
		CHECK_F(bytes_needed >= 0, "Bad string format: '%s'", format);
		char* buff = (char*)malloc(bytes_needed+1);
		vsnprintf(buff, bytes_needed+1, format, vlist);
		return Text(buff);
#else
		char* buff = nullptr;
		int result = vasprintf(&buff, format, vlist);
		CHECK_F(result >= 0, "Bad string format: '" LOGURU_FMT(s) "'", format);
		return Text(buff);
#endif
	}

	Text textprintf(const char* format, ...)
	{
		va_list vlist;
		va_start(vlist, format);
		auto result = vtextprintf(format, vlist);
		va_end(vlist);
		return result;
	}
#endif

	// Overloaded for variadic template matching.
	Text textprintf()
	{
		return Text(static_cast<char*>(calloc(1, 1)));
	}

	static const char* indentation(unsigned depth)
	{
		static const char buff[] =
		".   .   .   .   .   .   .   .   .   .   " ".   .   .   .   .   .   .   .   .   .   "
		".   .   .   .   .   .   .   .   .   .   " ".   .   .   .   .   .   .   .   .   .   "
		".   .   .   .   .   .   .   .   .   .   " ".   .   .   .   .   .   .   .   .   .   "
		".   .   .   .   .   .   .   .   .   .   " ".   .   .   .   .   .   .   .   .   .   "
		".   .   .   .   .   .   .   .   .   .   " ".   .   .   .   .   .   .   .   .   .   ";
		static const size_t INDENTATION_WIDTH = 4;
		static const size_t NUM_INDENTATIONS = (sizeof(buff) - 1) / INDENTATION_WIDTH;
		depth = std::min<unsigned>(depth, NUM_INDENTATIONS);
		return buff + INDENTATION_WIDTH * (NUM_INDENTATIONS - depth);
	}

	static void parse_args(int& argc, char* argv[], const char* verbosity_flag)
	{
		int arg_dest = 1;
		int out_argc = argc;

		for (int arg_it = 1; arg_it < argc; ++arg_it) {
			auto cmd = argv[arg_it];
			auto arg_len = strlen(verbosity_flag);

			bool last_is_alpha = false;
			#if LOGURU_USE_LOCALE
			try {  // locale variant of isalpha will throw on error
				last_is_alpha = std::isalpha(cmd[arg_len], std::locale(""));
			}
			catch (...) {
				last_is_alpha = std::isalpha(static_cast<int>(cmd[arg_len]));
			}
			#else
			last_is_alpha = std::isalpha(static_cast<int>(cmd[arg_len]));
			#endif

			if (strncmp(cmd, verbosity_flag, arg_len) == 0 && !last_is_alpha) {
				out_argc -= 1;
				auto value_str = cmd + arg_len;
				if (value_str[0] == '\0') {
					// Value in separate argument
					arg_it += 1;
					CHECK_LT_F(arg_it, argc, "Missing verbosiy level after " LOGURU_FMT(s) "", verbosity_flag);
					value_str = argv[arg_it];
					out_argc -= 1;
				}
				if (*value_str == '=') { value_str += 1; }

				auto req_verbosity = get_verbosity_from_name(value_str);
				if (req_verbosity != Verbosity_INVALID) {
					g_stderr_verbosity = req_verbosity;
				} else {
					char* end = 0;
					g_stderr_verbosity = static_cast<int>(strtol(value_str, &end, 10));
					CHECK_F(end && *end == '\0',
						"Invalid verbosity. Expected integer, INFO, WARNING, ERROR or OFF, got '" LOGURU_FMT(s) "'", value_str);
				}
			} else {
				argv[arg_dest++] = argv[arg_it];
			}
		}

		argc = out_argc;
		argv[argc] = nullptr;
	}

	static long long now_ns()
	{
		return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}

	// Returns the part of the path after the last / or \ (if any).
	const char* filename(const char* path)
	{
		for (auto ptr = path; *ptr; ++ptr) {
			if (*ptr == '/' || *ptr == '\\') {
				path = ptr + 1;
			}
		}
		return path;
	}

	// ------------------------------------------------------------------------------

	static void on_atexit()
	{
		VLOG_F(g_internal_verbosity, "atexit");
		flush();
	}

	static void install_signal_handlers(const SignalOptions& signal_options);

	static void write_hex_digit(std::string& out, unsigned num)
	{
		DCHECK_LT_F(num, 16u);
		if (num < 10u) { out.push_back(char('0' + num)); }
		else { out.push_back(char('A' + num - 10)); }
	}

	static void write_hex_byte(std::string& out, uint8_t n)
	{
		write_hex_digit(out, n >> 4u);
		write_hex_digit(out, n & 0x0f);
	}

	static void escape(std::string& out, const std::string& str)
	{
		for (char c : str) {
			/**/ if (c == '\a') { out += "\\a";  }
			else if (c == '\b') { out += "\\b";  }
			else if (c == '\f') { out += "\\f";  }
			else if (c == '\n') { out += "\\n";  }
			else if (c == '\r') { out += "\\r";  }
			else if (c == '\t') { out += "\\t";  }
			else if (c == '\v') { out += "\\v";  }
			else if (c == '\\') { out += "\\\\"; }
			else if (c == '\'') { out += "\\\'"; }
			else if (c == '\"') { out += "\\\""; }
			else if (c == ' ')  { out += "\\ ";  }
			else if (0 <= c && c < 0x20) { // ASCI control character:
			// else if (c < 0x20 || c != (c & 127)) { // ASCII control character or UTF-8:
				out += "\\x";
				write_hex_byte(out, static_cast<uint8_t>(c));
			} else { out += c; }
		}
	}

	Text errno_as_text()
	{
		char buff[256];
	#if defined(__GLIBC__) && defined(_GNU_SOURCE)
		// GNU Version
		return Text(STRDUP(strerror_r(errno, buff, sizeof(buff))));
	#elif defined(__APPLE__) || _POSIX_C_SOURCE >= 200112L
		// XSI Version
		strerror_r(errno, buff, sizeof(buff));
		return Text(strdup(buff));
	#elif defined(_WIN32)
		strerror_s(buff, sizeof(buff), errno);
		return Text(STRDUP(buff));
	#else
		// Not thread-safe.
		return Text(STRDUP(strerror(errno)));
	#endif
	}

	void init(int& argc, char* argv[], const Options& options)
	{
		CHECK_GT_F(argc,       0,       "Expected proper argc/argv");
		CHECK_EQ_F(argv[argc], nullptr, "Expected proper argc/argv");

		s_argv0_filename = filename(argv[0]);

		#ifdef _WIN32
			#define getcwd _getcwd
		#endif

		if (!getcwd(s_current_dir, sizeof(s_current_dir))) {
			const auto error_text = errno_as_text();
			LOG_F(WARNING, "Failed to get current working directory: " LOGURU_FMT(s) "", error_text.c_str());
		}

		s_arguments = "";
		for (int i = 0; i < argc; ++i) {
			escape(s_arguments, argv[i]);
			if (i + 1 < argc) {
				s_arguments += " ";
			}
		}

		if (options.verbosity_flag) {
			parse_args(argc, argv, options.verbosity_flag);
		}

		if (const auto main_thread_name = options.main_thread_name) {
			#if LOGURU_PTLS_NAMES || LOGURU_WINTHREADS
				set_thread_name(main_thread_name);
			#elif LOGURU_PTHREADS
				char old_thread_name[16] = {0};
				auto this_thread = pthread_self();
				#if defined(__APPLE__) || defined(__linux__) || defined(__sun)
					pthread_getname_np(this_thread, old_thread_name, sizeof(old_thread_name));
				#endif
				if (old_thread_name[0] == 0) {
					#ifdef __APPLE__
						pthread_setname_np(main_thread_name);
					#elif defined(__FreeBSD__) || defined(__OpenBSD__)
						pthread_set_name_np(this_thread, main_thread_name);
					#elif defined(__linux__) || defined(__sun)
						pthread_setname_np(this_thread, main_thread_name);
					#endif
				}
			#endif // LOGURU_PTHREADS
		}

		if (g_stderr_verbosity >= Verbosity_INFO) {
			if (g_preamble_header) {
				char preamble_explain[LOGURU_PREAMBLE_WIDTH];
				print_preamble_header(preamble_explain, sizeof(preamble_explain));
				if (g_colorlogtostderr && s_terminal_has_color) {
					fprintf(stderr, "%s%s%s\n", terminal_reset(), terminal_dim(), preamble_explain);
				} else {
					fprintf(stderr, "%s\n", preamble_explain);
				}
			}
			fflush(stderr);
		}
		VLOG_F(g_internal_verbosity, "arguments: " LOGURU_FMT(s) "", s_arguments.c_str());
		if (strlen(s_current_dir) != 0)
		{
			VLOG_F(g_internal_verbosity, "Current dir: " LOGURU_FMT(s) "", s_current_dir);
		}
		VLOG_F(g_internal_verbosity, "stderr verbosity: " LOGURU_FMT(d) "", g_stderr_verbosity);
		VLOG_F(g_internal_verbosity, "-----------------------------------");

		install_signal_handlers(options.signal_options);

		atexit(on_atexit);
	}

	void shutdown()
	{
		VLOG_F(g_internal_verbosity, "loguru::shutdown()");
		remove_all_callbacks();
		set_fatal_handler(nullptr);
		set_verbosity_to_name_callback(nullptr);
		set_name_to_verbosity_callback(nullptr);
	}

	void write_date_time(char* buff, unsigned long long buff_size)
	{
		auto now = system_clock::now();
		long long ms_since_epoch = duration_cast<milliseconds>(now.time_since_epoch()).count();
		time_t sec_since_epoch = time_t(ms_since_epoch / 1000);
		tm time_info;
		localtime_r(&sec_since_epoch, &time_info);
		snprintf(buff, buff_size, "%04d%02d%02d_%02d%02d%02d.%03lld",
			1900 + time_info.tm_year, 1 + time_info.tm_mon, time_info.tm_mday,
			time_info.tm_hour, time_info.tm_min, time_info.tm_sec, ms_since_epoch % 1000);
	}

	const char* argv0_filename()
	{
		return s_argv0_filename.c_str();
	}

	const char* arguments()
	{
		return s_arguments.c_str();
	}

	const char* current_dir()
	{
		return s_current_dir;
	}

	const char* home_dir()
	{
		#ifdef __MINGW32__
			auto home = getenv("USERPROFILE");
			CHECK_F(home != nullptr, "Missing USERPROFILE");
			return home;
		#elif defined(_WIN32)
			char* user_profile;
			size_t len;
			errno_t err = _dupenv_s(&user_profile, &len, "USERPROFILE");
			CHECK_F(err == 0, "Missing USERPROFILE");
			return user_profile;
		#else // _WIN32
			auto home = getenv("HOME");
			CHECK_F(home != nullptr, "Missing HOME");
			return home;
		#endif // _WIN32
	}

	void suggest_log_path(const char* prefix, char* buff, unsigned long long buff_size)
	{
		if (prefix[0] == '~') {
			snprintf(buff, buff_size - 1, "%s%s", home_dir(), prefix + 1);
		} else {
			snprintf(buff, buff_size - 1, "%s", prefix);
		}

		// Check for terminating /
		size_t n = strlen(buff);
		if (n != 0) {
			if (buff[n - 1] != '/') {
				CHECK_F(n + 2 < buff_size, "Filename buffer too small");
				buff[n] = '/';
				buff[n + 1] = '\0';
			}
		}

	#ifdef _WIN32
		strncat_s(buff, buff_size - strlen(buff) - 1, s_argv0_filename.c_str(), buff_size - strlen(buff) - 1);
		strncat_s(buff, buff_size - strlen(buff) - 1, "/",                      buff_size - strlen(buff) - 1);
		write_date_time(buff + strlen(buff),    buff_size - strlen(buff));
		strncat_s(buff, buff_size - strlen(buff) - 1, ".log",                   buff_size - strlen(buff) - 1);
	#else
		strncat(buff, s_argv0_filename.c_str(), buff_size - strlen(buff) - 1);
		strncat(buff, "/", buff_size - strlen(buff) - 1);
		write_date_time(buff + strlen(buff), buff_size - strlen(buff));
		strncat(buff, ".log", buff_size - strlen(buff) - 1);
	#endif
	}

	bool create_directories(const char* file_path_const)
	{
		CHECK_F(file_path_const && *file_path_const);
		char* file_path = STRDUP(file_path_const);
		for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
			*p = '\0';

	#ifdef _WIN32
			if (_mkdir(file_path) == -1) {
	#else
			if (mkdir(file_path, 0755) == -1) {
	#endif
				if (errno != EEXIST) {
					LOG_F(ERROR, "Failed to create directory '" LOGURU_FMT(s) "'", file_path);
					LOG_IF_F(ERROR, errno == EACCES,       "EACCES");
					LOG_IF_F(ERROR, errno == ENAMETOOLONG, "ENAMETOOLONG");
					LOG_IF_F(ERROR, errno == ENOENT,       "ENOENT");
					LOG_IF_F(ERROR, errno == ENOTDIR,      "ENOTDIR");
					LOG_IF_F(ERROR, errno == ELOOP,        "ELOOP");

					*p = '/';
					free(file_path);
					return false;
				}
			}
			*p = '/';
		}
		free(file_path);
		return true;
	}
	bool add_file(const char* path_in, FileMode mode, Verbosity verbosity)
	{
		char path[PATH_MAX];
		if (path_in[0] == '~') {
			snprintf(path, sizeof(path) - 1, "%s%s", home_dir(), path_in + 1);
		} else {
			snprintf(path, sizeof(path) - 1, "%s", path_in);
		}

		if (!create_directories(path)) {
			LOG_F(ERROR, "Failed to create directories to '" LOGURU_FMT(s) "'", path);
		}

		const char* mode_str = (mode == FileMode::Truncate ? "w" : "a");
		FILE* file;
	#ifdef _WIN32
		file = _fsopen(path, mode_str, _SH_DENYNO);
	#else
		file = fopen(path, mode_str);
	#endif
		if (!file) {
			LOG_F(ERROR, "Failed to open '" LOGURU_FMT(s) "'", path);
			return false;
		}
#if LOGURU_WITH_FILEABS
		FileAbs* file_abs = new FileAbs(); // this is deleted in file_close;
		snprintf(file_abs->path, sizeof(file_abs->path) - 1, "%s", path);
		snprintf(file_abs->mode_str, sizeof(file_abs->mode_str) - 1, "%s", mode_str);
		stat(file_abs->path, &file_abs->st);
		file_abs->fp = file;
		file_abs->verbosity = verbosity;
		add_callback(path_in, file_log, file_abs, verbosity, file_close, file_flush);
#else
		add_callback(path_in, file_log, file, verbosity, file_close, file_flush);
#endif

		if (mode == FileMode::Append) {
			fprintf(file, "\n\n\n\n\n");
		}
		if (!s_arguments.empty()) {
			fprintf(file, "arguments: %s\n", s_arguments.c_str());
		}
		if (strlen(s_current_dir) != 0) {
			fprintf(file, "Current dir: %s\n", s_current_dir);
		}
		fprintf(file, "File verbosity level: %d\n", verbosity);
		if (g_preamble_header) {
			char preamble_explain[LOGURU_PREAMBLE_WIDTH];
			print_preamble_header(preamble_explain, sizeof(preamble_explain));
			fprintf(file, "%s\n", preamble_explain);
		}
		fflush(file);

		VLOG_F(g_internal_verbosity, "Logging to '" LOGURU_FMT(s) "', mode: '" LOGURU_FMT(s) "', verbosity: " LOGURU_FMT(d) "", path, mode_str, verbosity);
		return true;
	}

	/*
		Will add syslog as a standard sink for log messages
		Any logging message with a verbosity lower or equal to
		the given verbosity will be included.

		This works for Unix like systems (i.e. Linux/Mac)
		There is no current implementation for Windows (as I don't know the
		equivalent calls or have a way to test them). If you know please
		add and send a pull request.

		The code should still compile under windows but will only generate
		a warning message that syslog is unavailable.

		Search for LOGURU_SYSLOG to find and fix.
	*/
	bool add_syslog(const char* app_name, Verbosity verbosity)
	{
		return add_syslog(app_name, verbosity, LOG_USER);
	}
	bool add_syslog(const char* app_name, Verbosity verbosity, int facility)
	{
#if LOGURU_SYSLOG
		if (app_name == nullptr) {
			app_name = argv0_filename();
		}
		openlog(app_name, 0, facility);
		add_callback("'syslog'", syslog_log, nullptr, verbosity, syslog_close, syslog_flush);

		VLOG_F(g_internal_verbosity, "Logging to 'syslog' , verbosity: " LOGURU_FMT(d) "", verbosity);
		return true;
#else
		(void)app_name;
		(void)verbosity;
		(void)facility;
		VLOG_F(g_internal_verbosity, "syslog not implemented on this system. Request to install syslog logging ignored.");
		return false;
#endif
	}
	// Will be called right before abort().
	void set_fatal_handler(fatal_handler_t handler)
	{
		s_fatal_handler = handler;
	}

	fatal_handler_t get_fatal_handler()
	{
		return s_fatal_handler;
	}

	void set_verbosity_to_name_callback(verbosity_to_name_t callback)
	{
		s_verbosity_to_name_callback = callback;
	}

	void set_name_to_verbosity_callback(name_to_verbosity_t callback)
	{
		s_name_to_verbosity_callback = callback;
	}

	void add_stack_cleanup(const char* find_this, const char* replace_with_this)
	{
		if (strlen(find_this) <= strlen(replace_with_this)) {
			LOG_F(WARNING, "add_stack_cleanup: the replacement should be shorter than the pattern!");
			return;
		}

		s_user_stack_cleanups.push_back(StringPair(find_this, replace_with_this));
	}

	static void on_callback_change()
	{
		s_max_out_verbosity = Verbosity_OFF;
		for (const auto& callback : s_callbacks) {
			s_max_out_verbosity = std::max(s_max_out_verbosity, callback.verbosity);
		}
	}

	void add_callback(
		const char*     id,
		log_handler_t   callback,
		void*           user_data,
		Verbosity       verbosity,
		close_handler_t on_close,
		flush_handler_t on_flush)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		s_callbacks.push_back(Callback{id, callback, user_data, verbosity, on_close, on_flush, 0});
		on_callback_change();
	}

	// Returns a custom verbosity name if one is available, or nullptr.
	// See also set_verbosity_to_name_callback.
	const char* get_verbosity_name(Verbosity verbosity)
	{
		auto name = s_verbosity_to_name_callback
				? (*s_verbosity_to_name_callback)(verbosity)
				: nullptr;

		// Use standard replacements if callback fails:
		if (!name)
		{
			if (verbosity <= Verbosity_FATAL) {
				name = "FATL";
			} else if (verbosity == Verbosity_ERROR) {
				name = "ERR";
			} else if (verbosity == Verbosity_WARNING) {
				name = "WARN";
			} else if (verbosity == Verbosity_INFO) {
				name = "INFO";
			}
		}

		return name;
	}

	// Returns Verbosity_INVALID if the name is not found.
	// See also set_name_to_verbosity_callback.
	Verbosity get_verbosity_from_name(const char* name)
	{
		auto verbosity = s_name_to_verbosity_callback
				? (*s_name_to_verbosity_callback)(name)
				: Verbosity_INVALID;

		// Use standard replacements if callback fails:
		if (verbosity == Verbosity_INVALID) {
			if (strcmp(name, "OFF") == 0) {
				verbosity = Verbosity_OFF;
			} else if (strcmp(name, "INFO") == 0) {
				verbosity = Verbosity_INFO;
			} else if (strcmp(name, "WARNING") == 0) {
				verbosity = Verbosity_WARNING;
			} else if (strcmp(name, "ERROR") == 0) {
				verbosity = Verbosity_ERROR;
			} else if (strcmp(name, "FATAL") == 0) {
				verbosity = Verbosity_FATAL;
			}
		}

		return verbosity;
	}

	bool remove_callback(const char* id)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		auto it = std::find_if(begin(s_callbacks), end(s_callbacks), [&](const Callback& c) { return c.id == id; });
		if (it != s_callbacks.end()) {
			if (it->close) { it->close(it->user_data); }
			s_callbacks.erase(it);
			on_callback_change();
			return true;
		} else {
			LOG_F(ERROR, "Failed to locate callback with id '" LOGURU_FMT(s) "'", id);
			return false;
		}
	}

	void remove_all_callbacks()
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		for (auto& callback : s_callbacks) {
			if (callback.close) {
				callback.close(callback.user_data);
			}
		}
		s_callbacks.clear();
		on_callback_change();
	}

	// Returns the maximum of g_stderr_verbosity and all file/custom outputs.
	Verbosity current_verbosity_cutoff()
	{
		return g_stderr_verbosity > s_max_out_verbosity ?
			   g_stderr_verbosity : s_max_out_verbosity;
	}

	// ------------------------------------------------------------------------
	// Threads names

#if LOGURU_PTLS_NAMES
	static pthread_once_t s_pthread_key_once = PTHREAD_ONCE_INIT;
	static pthread_key_t  s_pthread_key_name;

	void make_pthread_key_name()
	{
		(void)pthread_key_create(&s_pthread_key_name, free);
	}
#endif

#if LOGURU_WINTHREADS
	// Where we store the custom thread name set by `set_thread_name`
	char* thread_name_buffer()
	{
		__declspec( thread ) static char thread_name[LOGURU_THREADNAME_WIDTH + 1] = {0};
		return &thread_name[0];
	}
#endif // LOGURU_WINTHREADS

	void set_thread_name(const char* name)
	{
		#if LOGURU_PTLS_NAMES
			// Store thread name in thread-local storage at `s_pthread_key_name`
			(void)pthread_once(&s_pthread_key_once, make_pthread_key_name);
			(void)pthread_setspecific(s_pthread_key_name, STRDUP(name));
		#elif LOGURU_PTHREADS
			// Tell the OS the thread name
			#ifdef __APPLE__
				pthread_setname_np(name);
			#elif defined(__FreeBSD__) || defined(__OpenBSD__)
				pthread_set_name_np(pthread_self(), name);
			#elif defined(__linux__) || defined(__sun)
				pthread_setname_np(pthread_self(), name);
			#endif
		#elif LOGURU_WINTHREADS
			// Store thread name in a thread-local storage:
			strncpy_s(thread_name_buffer(), LOGURU_THREADNAME_WIDTH + 1, name, _TRUNCATE);
		#else // LOGURU_PTHREADS
			// TODO: on these weird platforms we should also store the thread name
			// in a generic thread-local storage.
			(void)name;
		#endif // LOGURU_PTHREADS
	}

	void get_thread_name(char* buffer, unsigned long long length, bool right_align_hex_id)
	{
		CHECK_NE_F(length, 0u, "Zero length buffer in get_thread_name");
		CHECK_NOTNULL_F(buffer, "nullptr in get_thread_name");

		#if LOGURU_PTLS_NAMES
			(void)pthread_once(&s_pthread_key_once, make_pthread_key_name);
			if (const char* name = static_cast<const char*>(pthread_getspecific(s_pthread_key_name))) {
				snprintf(buffer, static_cast<size_t>(length), "%s", name);
			} else {
				buffer[0] = 0;
			}
		#elif LOGURU_PTHREADS
			// Ask the OS about the thread name.
			// This is what we *want* to do on all platforms, but
			// only some platforms support it (currently).
			pthread_getname_np(pthread_self(), buffer, length);
		#elif LOGURU_WINTHREADS
			snprintf(buffer, static_cast<size_t>(length), "%s", thread_name_buffer());
		#else
			// Thread names unsupported
			buffer[0] = 0;
		#endif

		if (buffer[0] == 0) {
			// We failed to get a readable thread name.
			// Write a HEX thread ID instead.
			// We try to get an ID that is the same as the ID you could
			// read in your debugger, system monitor etc.

			#ifdef __APPLE__
				uint64_t thread_id;
				pthread_threadid_np(pthread_self(), &thread_id);
			#elif defined(__FreeBSD__)
				long thread_id;
				(void)thr_self(&thread_id);
			#elif LOGURU_PTHREADS
				uint64_t thread_id = pthread_self();
			#else
				// This ID does not correllate to anything we can get from the OS,
				// so this is the worst way to get the ID.
				const auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
			#endif

			if (right_align_hex_id) {
				snprintf(buffer, static_cast<size_t>(length), "%*X", static_cast<int>(length - 1), static_cast<unsigned>(thread_id));
			} else {
				snprintf(buffer, static_cast<size_t>(length), "%X", static_cast<unsigned>(thread_id));
			}
		}
	}

	// ------------------------------------------------------------------------
	// Stack traces

#if LOGURU_STACKTRACES
	Text demangle(const char* name)
	{
		int status = -1;
		char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
		Text result{status == 0 ? demangled : STRDUP(name)};
		return result;
	}

	#if LOGURU_RTTI
		template <class T>
		std::string type_name()
		{
			auto demangled = demangle(typeid(T).name());
			return demangled.c_str();
		}
	#endif // LOGURU_RTTI

	static const StringPairList REPLACE_LIST = {
		#if LOGURU_RTTI
			{ type_name<std::string>(),    "std::string"    },
			{ type_name<std::wstring>(),   "std::wstring"   },
			{ type_name<std::u16string>(), "std::u16string" },
			{ type_name<std::u32string>(), "std::u32string" },
		#endif // LOGURU_RTTI
		{ "std::__1::",                "std::"          },
		{ "__thiscall ",               ""               },
		{ "__cdecl ",                  ""               },
	};

	void do_replacements(const StringPairList& replacements, std::string& str)
	{
		for (auto&& p : replacements) {
			if (p.first.size() <= p.second.size()) {
				// On gcc, "type_name<std::string>()" is "std::string"
				continue;
			}

			size_t it;
			while ((it=str.find(p.first)) != std::string::npos) {
				str.replace(it, p.first.size(), p.second);
			}
		}
	}

	std::string prettify_stacktrace(const std::string& input)
	{
		std::string output = input;

		do_replacements(s_user_stack_cleanups, output);
		do_replacements(REPLACE_LIST, output);

		try {
			std::regex std_allocator_re(R"(,\s*std::allocator<[^<>]+>)");
			output = std::regex_replace(output, std_allocator_re, std::string(""));

			std::regex template_spaces_re(R"(<\s*([^<> ]+)\s*>)");
			output = std::regex_replace(output, template_spaces_re, std::string("<$1>"));
		} catch (std::regex_error&) {
			// Probably old GCC.
		}

		return output;
	}

	std::string stacktrace_as_stdstring(int skip)
	{
		// From https://gist.github.com/fmela/591333
		void* callstack[128];
		const auto max_frames = sizeof(callstack) / sizeof(callstack[0]);
		int num_frames = backtrace(callstack, max_frames);
		char** symbols = backtrace_symbols(callstack, num_frames);

		std::string result;
		// Print stack traces so the most relevant ones are written last
		// Rationale: http://yellerapp.com/posts/2015-01-22-upside-down-stacktraces.html
		for (int i = num_frames - 1; i >= skip; --i) {
			char buf[1024];
			Dl_info info;
			if (dladdr(callstack[i], &info) && info.dli_sname) {
				char* demangled = NULL;
				int status = -1;
				if (info.dli_sname[0] == '_') {
					demangled = abi::__cxa_demangle(info.dli_sname, 0, 0, &status);
				}
				snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
						 i - skip, int(2 + sizeof(void*) * 2), callstack[i],
						 status == 0 ? demangled :
						 info.dli_sname == 0 ? symbols[i] : info.dli_sname,
						 static_cast<char*>(callstack[i]) - static_cast<char*>(info.dli_saddr));
				free(demangled);
			} else {
				snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
						 i - skip, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
			}
			result += buf;
		}
		free(symbols);

		if (num_frames == max_frames) {
			result = "[truncated]\n" + result;
		}

		if (!result.empty() && result[result.size() - 1] == '\n') {
			result.resize(result.size() - 1);
		}

		return prettify_stacktrace(result);
	}

#else // LOGURU_STACKTRACES
	Text demangle(const char* name)
	{
		return Text(STRDUP(name));
	}

	std::string stacktrace_as_stdstring(int)
	{
		// No stacktraces available on this platform"
		return "";
	}

#endif // LOGURU_STACKTRACES

	Text stacktrace(int skip)
	{
		auto str = stacktrace_as_stdstring(skip + 1);
		return Text(STRDUP(str.c_str()));
	}

	// ------------------------------------------------------------------------

	static void print_preamble_header(char* out_buff, size_t out_buff_size)
	{
		if (out_buff_size == 0) { return; }
		out_buff[0] = '\0';
		size_t pos = 0;
		if (g_preamble_date && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "date       ");
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_time && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "time         ");
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_uptime && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "( uptime  ) ");
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_thread && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "[%-*s]", LOGURU_THREADNAME_WIDTH, " thread name/id");
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_file && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%*s:line  ", LOGURU_FILENAME_WIDTH, "file");
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_verbose && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "   v");
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_pipe && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "| ");
			if (bytes > 0) {
				pos += bytes;
			}
		}
	}

	static void print_preamble(char* out_buff, size_t out_buff_size, Verbosity verbosity, const char* file, unsigned line)
	{
		if (out_buff_size == 0) { return; }
		out_buff[0] = '\0';
		if (!g_preamble) { return; }
		long long ms_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		time_t sec_since_epoch = time_t(ms_since_epoch / 1000);
		tm time_info;
		localtime_r(&sec_since_epoch, &time_info);

		auto uptime_ms = duration_cast<milliseconds>(steady_clock::now() - s_start_time).count();
		auto uptime_sec = static_cast<double> (uptime_ms) / 1000.0;

		char thread_name[LOGURU_THREADNAME_WIDTH + 1] = {0};
		get_thread_name(thread_name, LOGURU_THREADNAME_WIDTH + 1, true);

		if (s_strip_file_path) {
			file = filename(file);
		}

		char level_buff[6];
		const char* custom_level_name = get_verbosity_name(verbosity);
		if (custom_level_name) {
			snprintf(level_buff, sizeof(level_buff) - 1, "%s", custom_level_name);
		} else {
			snprintf(level_buff, sizeof(level_buff) - 1, "% 4d", static_cast<int8_t>(verbosity));
		}

		size_t pos = 0;

		if (g_preamble_date && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%04d-%02d-%02d ",
				                 1900 + time_info.tm_year, 1 + time_info.tm_mon, time_info.tm_mday);
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_time && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%02d:%02d:%02d.%03lld ",
			                     time_info.tm_hour, time_info.tm_min, time_info.tm_sec, ms_since_epoch % 1000);
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_uptime && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "(%8.3fs) ",
			                     uptime_sec);
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_thread && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "[%-*s]",
			                     LOGURU_THREADNAME_WIDTH, thread_name);
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_file && pos < out_buff_size) {
			char shortened_filename[LOGURU_FILENAME_WIDTH + 1];
			snprintf(shortened_filename, LOGURU_FILENAME_WIDTH + 1, "%s", file);
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%*s:%-5u ",
			                     LOGURU_FILENAME_WIDTH, shortened_filename, line);
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_verbose && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%4s",
			                     level_buff);
			if (bytes > 0) {
				pos += bytes;
			}
		}
		if (g_preamble_pipe && pos < out_buff_size) {
			int bytes = snprintf(out_buff + pos, out_buff_size - pos, "| ");
			if (bytes > 0) {
				pos += bytes;
			}
		}
	}

	// stack_trace_skip is just if verbosity == FATAL.
	static void log_message(int stack_trace_skip, Message& message, bool with_indentation, bool abort_if_fatal)
	{
		const auto verbosity = message.verbosity;
		std::lock_guard<std::recursive_mutex> lock(s_mutex);

		if (message.verbosity == Verbosity_FATAL) {
			auto st = loguru::stacktrace(stack_trace_skip + 2);
			if (!st.empty()) {
				RAW_LOG_F(ERROR, "Stack trace:\n" LOGURU_FMT(s) "", st.c_str());
			}

			auto ec = loguru::get_error_context();
			if (!ec.empty()) {
				RAW_LOG_F(ERROR, "" LOGURU_FMT(s) "", ec.c_str());
			}
		}

		if (with_indentation) {
			message.indentation = indentation(s_stderr_indentation);
		}

		if (verbosity <= g_stderr_verbosity) {
			if (g_colorlogtostderr && s_terminal_has_color) {
				if (verbosity > Verbosity_WARNING) {
					fprintf(stderr, "%s%s%s%s%s%s%s%s\n",
						terminal_reset(),
						terminal_dim(),
						message.preamble,
						message.indentation,
						verbosity == Verbosity_INFO ? terminal_reset() : "", // un-dim for info
						message.prefix,
						message.message,
						terminal_reset());
				} else {
					fprintf(stderr, "%s%s%s%s%s%s%s\n",
						terminal_reset(),
						verbosity == Verbosity_WARNING ? terminal_yellow() : terminal_red(),
						message.preamble,
						message.indentation,
						message.prefix,
						message.message,
						terminal_reset());
				}
			} else {
				fprintf(stderr, "%s%s%s%s\n",
					message.preamble, message.indentation, message.prefix, message.message);
			}

			if (g_flush_interval_ms == 0) {
				fflush(stderr);
			} else {
				s_needs_flushing = true;
			}
		}

		for (auto& p : s_callbacks) {
			if (verbosity <= p.verbosity) {
				if (with_indentation) {
					message.indentation = indentation(p.indentation);
				}
				p.callback(p.user_data, message);
				if (g_flush_interval_ms == 0) {
					if (p.flush) { p.flush(p.user_data); }
				} else {
					s_needs_flushing = true;
				}
			}
		}

		if (g_flush_interval_ms > 0 && !s_flush_thread) {
			s_flush_thread = new std::thread([](){
				for (;;) {
					if (s_needs_flushing) {
						flush();
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(g_flush_interval_ms));
				}
			});
		}

		if (message.verbosity == Verbosity_FATAL) {
			flush();

			if (s_fatal_handler) {
				s_fatal_handler(message);
				flush();
			}

			if (abort_if_fatal) {
#if !defined(_WIN32)
				if (s_signal_options.sigabrt) {
					// Make sure we don't catch our own abort:
					signal(SIGABRT, SIG_DFL);
				}
#endif
				abort();
			}
		}
	}

	// stack_trace_skip is just if verbosity == FATAL.
	void log_to_everywhere(int stack_trace_skip, Verbosity verbosity,
	                       const char* file, unsigned line,
	                       const char* prefix, const char* buff)
	{
		char preamble_buff[LOGURU_PREAMBLE_WIDTH];
		print_preamble(preamble_buff, sizeof(preamble_buff), verbosity, file, line);
		auto message = Message{verbosity, file, line, preamble_buff, "", prefix, buff};
		log_message(stack_trace_skip + 1, message, true, true);
	}

#if LOGURU_USE_FMTLIB
	void vlog(Verbosity verbosity, const char* file, unsigned line, const char* format, fmt::format_args args)
	{
		auto formatted = fmt::vformat(format, args);
		log_to_everywhere(1, verbosity, file, line, "", formatted.c_str());
	}

	void raw_vlog(Verbosity verbosity, const char* file, unsigned line, const char* format, fmt::format_args args)
	{
		auto formatted = fmt::vformat(format, args);
		auto message = Message{verbosity, file, line, "", "", "", formatted.c_str()};
		log_message(1, message, false, true);
	}
#else
	void log(Verbosity verbosity, const char* file, unsigned line, const char* format, ...)
	{
		va_list vlist;
		va_start(vlist, format);
		vlog(verbosity, file, line, format, vlist);
		va_end(vlist);
	}

	void vlog(Verbosity verbosity, const char* file, unsigned line, const char* format, va_list vlist)
	{
		auto buff = vtextprintf(format, vlist);
		log_to_everywhere(1, verbosity, file, line, "", buff.c_str());
	}

	void raw_log(Verbosity verbosity, const char* file, unsigned line, const char* format, ...)
	{
		va_list vlist;
		va_start(vlist, format);
		auto buff = vtextprintf(format, vlist);
		auto message = Message{verbosity, file, line, "", "", "", buff.c_str()};
		log_message(1, message, false, true);
		va_end(vlist);
	}
#endif

	void flush()
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		fflush(stderr);
		for (const auto& callback : s_callbacks)
		{
			if (callback.flush) {
				callback.flush(callback.user_data);
			}
		}
		s_needs_flushing = false;
	}

	LogScopeRAII::LogScopeRAII(Verbosity verbosity, const char* file, unsigned line, const char* format, va_list vlist) :
		_verbosity(verbosity), _file(file), _line(line)
	{
		this->Init(format, vlist);
	}

	LogScopeRAII::LogScopeRAII(Verbosity verbosity, const char* file, unsigned line, const char* format, ...) :
		_verbosity(verbosity), _file(file), _line(line)
	{
		va_list vlist;
		va_start(vlist, format);
		this->Init(format, vlist);
		va_end(vlist);
	}

	LogScopeRAII::~LogScopeRAII()
	{
		if (_file) {
			std::lock_guard<std::recursive_mutex> lock(s_mutex);
			if (_indent_stderr && s_stderr_indentation > 0) {
				--s_stderr_indentation;
			}
			for (auto& p : s_callbacks) {
				// Note: Callback indentation cannot change!
				if (_verbosity <= p.verbosity) {
					// in unlikely case this callback is new
					if (p.indentation > 0) {
						--p.indentation;
					}
				}
			}
#if LOGURU_VERBOSE_SCOPE_ENDINGS
			auto duration_sec = static_cast<double>(now_ns() - _start_time_ns) / 1e9;
#if LOGURU_USE_FMTLIB
			auto buff = textprintf("{:.{}f} s: {:s}", duration_sec, LOGURU_SCOPE_TIME_PRECISION, _name);
#else
			auto buff = textprintf("%.*f s: %s", LOGURU_SCOPE_TIME_PRECISION, duration_sec, _name);
#endif
			log_to_everywhere(1, _verbosity, _file, _line, "} ", buff.c_str());
#else
			log_to_everywhere(1, _verbosity, _file, _line, "}", "");
#endif
		}
	}

	void LogScopeRAII::Init(const char* format, va_list vlist)
	{
		if (_verbosity <= current_verbosity_cutoff()) {
			std::lock_guard<std::recursive_mutex> lock(s_mutex);
			_indent_stderr = (_verbosity <= g_stderr_verbosity);
			_start_time_ns = now_ns();
			vsnprintf(_name, sizeof(_name), format, vlist);
			log_to_everywhere(1, _verbosity, _file, _line, "{ ", _name);

			if (_indent_stderr) {
				++s_stderr_indentation;
			}

			for (auto& p : s_callbacks) {
				if (_verbosity <= p.verbosity) {
					++p.indentation;
				}
			}
		} else {
			_file = nullptr;
		}
	}

#if LOGURU_USE_FMTLIB
	void vlog_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line, const char* format, fmt::format_args args)
	{
		auto formatted = fmt::vformat(format, args);
		log_to_everywhere(stack_trace_skip + 1, Verbosity_FATAL, file, line, expr, formatted.c_str());
		abort(); // log_to_everywhere already does this, but this makes the analyzer happy.
	}
#else
	void log_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line, const char* format, ...)
	{
		va_list vlist;
		va_start(vlist, format);
		auto buff = vtextprintf(format, vlist);
		log_to_everywhere(stack_trace_skip + 1, Verbosity_FATAL, file, line, expr, buff.c_str());
		va_end(vlist);
		abort(); // log_to_everywhere already does this, but this makes the analyzer happy.
	}
#endif

	void log_and_abort(int stack_trace_skip, const char* expr, const char* file, unsigned line)
	{
		log_and_abort(stack_trace_skip + 1, expr, file, line, " ");
	}

	// ----------------------------------------------------------------------------
	// Streams:

#if LOGURU_USE_FMTLIB
	template<typename... Args>
	std::string vstrprintf(const char* format, const Args&... args)
	{
		auto text = textprintf(format, args...);
		std::string result = text.c_str();
		return result;
	}

	template<typename... Args>
	std::string strprintf(const char* format, const Args&... args)
	{
		return vstrprintf(format, args...);
	}
#else
	std::string vstrprintf(const char* format, va_list vlist)
	{
		auto text = vtextprintf(format, vlist);
		std::string result = text.c_str();
		return result;
	}

	std::string strprintf(const char* format, ...)
	{
		va_list vlist;
		va_start(vlist, format);
		auto result = vstrprintf(format, vlist);
		va_end(vlist);
		return result;
	}
#endif

	#if LOGURU_WITH_STREAMS

	StreamLogger::~StreamLogger() noexcept(false)
	{
		auto message = _ss.str();
		log(_verbosity, _file, _line, LOGURU_FMT(s), message.c_str());
	}

	AbortLogger::~AbortLogger() noexcept(false)
	{
		auto message = _ss.str();
		loguru::log_and_abort(1, _expr, _file, _line, LOGURU_FMT(s), message.c_str());
	}

	#endif // LOGURU_WITH_STREAMS

	// ----------------------------------------------------------------------------
	// 888888 88""Yb 88""Yb  dP"Yb  88""Yb      dP""b8  dP"Yb  88b 88 888888 888888 Yb  dP 888888
	// 88__   88__dP 88__dP dP   Yb 88__dP     dP   `" dP   Yb 88Yb88   88   88__    YbdP    88
	// 88""   88"Yb  88"Yb  Yb   dP 88"Yb      Yb      Yb   dP 88 Y88   88   88""    dPYb    88
	// 888888 88  Yb 88  Yb  YbodP  88  Yb      YboodP  YbodP  88  Y8   88   888888 dP  Yb   88
	// ----------------------------------------------------------------------------

	struct StringStream
	{
		std::string str;
	};

	// Use this in your EcPrinter implementations.
	void stream_print(StringStream& out_string_stream, const char* text)
	{
		out_string_stream.str += text;
	}

	// ----------------------------------------------------------------------------

	using ECPtr = EcEntryBase*;

#if defined(_WIN32) || (defined(__APPLE__) && !TARGET_OS_IPHONE)
	#ifdef __APPLE__
		#define LOGURU_THREAD_LOCAL __thread
	#else
		#define LOGURU_THREAD_LOCAL thread_local
	#endif
	static LOGURU_THREAD_LOCAL ECPtr thread_ec_ptr = nullptr;

	ECPtr& get_thread_ec_head_ref()
	{
		return thread_ec_ptr;
	}
#else // !thread_local
	static pthread_once_t s_ec_pthread_once = PTHREAD_ONCE_INIT;
	static pthread_key_t  s_ec_pthread_key;

	void free_ec_head_ref(void* io_error_context)
	{
		delete reinterpret_cast<ECPtr*>(io_error_context);
	}

	void ec_make_pthread_key()
	{
		(void)pthread_key_create(&s_ec_pthread_key, free_ec_head_ref);
	}

	ECPtr& get_thread_ec_head_ref()
	{
		(void)pthread_once(&s_ec_pthread_once, ec_make_pthread_key);
		auto ec = reinterpret_cast<ECPtr*>(pthread_getspecific(s_ec_pthread_key));
		if (ec == nullptr) {
			ec = new ECPtr(nullptr);
			(void)pthread_setspecific(s_ec_pthread_key, ec);
		}
		return *ec;
	}
#endif // !thread_local

	// ----------------------------------------------------------------------------

	EcHandle get_thread_ec_handle()
	{
		return get_thread_ec_head_ref();
	}

	Text get_error_context()
	{
		return get_error_context_for(get_thread_ec_head_ref());
	}

	Text get_error_context_for(const EcEntryBase* ec_head)
	{
		std::vector<const EcEntryBase*> stack;
		while (ec_head) {
			stack.push_back(ec_head);
			ec_head = ec_head->_previous;
		}
		std::reverse(stack.begin(), stack.end());

		StringStream result;
		if (!stack.empty()) {
			result.str += "------------------------------------------------\n";
			for (auto entry : stack) {
				const auto description = std::string(entry->_descr) + ":";
#if LOGURU_USE_FMTLIB
				auto prefix = textprintf("[ErrorContext] {.{}s}:{:-5u} {:-20s} ",
					filename(entry->_file), LOGURU_FILENAME_WIDTH, entry->_line, description.c_str());
#else
				auto prefix = textprintf("[ErrorContext] %*s:%-5u %-20s ",
					LOGURU_FILENAME_WIDTH, filename(entry->_file), entry->_line, description.c_str());
#endif
				result.str += prefix.c_str();
				entry->print_value(result);
				result.str += "\n";
			}
			result.str += "------------------------------------------------";
		}
		return Text(STRDUP(result.str.c_str()));
	}

	EcEntryBase::EcEntryBase(const char* file, unsigned line, const char* descr)
		: _file(file), _line(line), _descr(descr)
	{
		EcEntryBase*& ec_head = get_thread_ec_head_ref();
		_previous = ec_head;
		ec_head = this;
	}

	EcEntryBase::~EcEntryBase()
	{
		get_thread_ec_head_ref() = _previous;
	}

	// ------------------------------------------------------------------------

	Text ec_to_text(const char* value)
	{
		// Add quotes around the string to make it obvious where it begin and ends.
		// This is great for detecting erroneous leading or trailing spaces in e.g. an identifier.
		auto str = "\"" + std::string(value) + "\"";
		return Text{STRDUP(str.c_str())};
	}

	Text ec_to_text(char c)
	{
		// Add quotes around the character to make it obvious where it begin and ends.
		std::string str = "'";

		auto write_hex_digit = [&](unsigned num)
		{
			if (num < 10u) { str += char('0' + num); }
			else           { str += char('a' + num - 10); }
		};

		auto write_hex_16 = [&](uint16_t n)
		{
			write_hex_digit((n >> 12u) & 0x0f);
			write_hex_digit((n >>  8u) & 0x0f);
			write_hex_digit((n >>  4u) & 0x0f);
			write_hex_digit((n >>  0u) & 0x0f);
		};

		if      (c == '\\') { str += "\\\\"; }
		else if (c == '\"') { str += "\\\""; }
		else if (c == '\'') { str += "\\\'"; }
		else if (c == '\0') { str += "\\0";  }
		else if (c == '\b') { str += "\\b";  }
		else if (c == '\f') { str += "\\f";  }
		else if (c == '\n') { str += "\\n";  }
		else if (c == '\r') { str += "\\r";  }
		else if (c == '\t') { str += "\\t";  }
		else if (0 <= c && c < 0x20) {
			str += "\\u";
			write_hex_16(static_cast<uint16_t>(c));
		} else { str += c; }

		str += "'";

		return Text{STRDUP(str.c_str())};
	}

	#define DEFINE_EC(Type)                        \
		Text ec_to_text(Type value)                \
		{                                          \
			auto str = std::to_string(value);      \
			return Text{STRDUP(str.c_str())};      \
		}

	DEFINE_EC(int)
	DEFINE_EC(unsigned int)
	DEFINE_EC(long)
	DEFINE_EC(unsigned long)
	DEFINE_EC(long long)
	DEFINE_EC(unsigned long long)
	DEFINE_EC(float)
	DEFINE_EC(double)
	DEFINE_EC(long double)

	#undef DEFINE_EC

	Text ec_to_text(EcHandle ec_handle)
	{
		Text parent_ec = get_error_context_for(ec_handle);
		size_t buffer_size = strlen(parent_ec.c_str()) + 2;
		char* with_newline = reinterpret_cast<char*>(malloc(buffer_size));
		with_newline[0] = '\n';
	#ifdef _WIN32
		strncpy_s(with_newline + 1, buffer_size, parent_ec.c_str(), buffer_size - 2);
	#else
		strcpy(with_newline + 1, parent_ec.c_str());
	#endif
		return Text(with_newline);
	}

	// ----------------------------------------------------------------------------

} // namespace loguru

// ----------------------------------------------------------------------------
// .dP"Y8 88  dP""b8 88b 88    db    88     .dP"Y8
// `Ybo." 88 dP   `" 88Yb88   dPYb   88     `Ybo."
// o.`Y8b 88 Yb  "88 88 Y88  dP__Yb  88  .o o.`Y8b
// 8bodP' 88  YboodP 88  Y8 dP""""Yb 88ood8 8bodP'
// ----------------------------------------------------------------------------

#ifdef _WIN32
namespace loguru {
	void install_signal_handlers(const SignalOptions& signal_options)
	{
		(void)signal_options;
		// TODO: implement signal handlers on windows
	}
} // namespace loguru

#else // _WIN32

namespace loguru
{
	void write_to_stderr(const char* data, size_t size)
	{
		auto result = write(STDERR_FILENO, data, size);
		(void)result; // Ignore errors.
	}

	void write_to_stderr(const char* data)
	{
		write_to_stderr(data, strlen(data));
	}

	void call_default_signal_handler(int signal_number)
	{
		struct sigaction sig_action;
		memset(&sig_action, 0, sizeof(sig_action));
		sigemptyset(&sig_action.sa_mask);
		sig_action.sa_handler = SIG_DFL;
		sigaction(signal_number, &sig_action, NULL);
		kill(getpid(), signal_number);
	}

	void signal_handler(int signal_number, siginfo_t*, void*)
	{
		const char* signal_name = "UNKNOWN SIGNAL";

		if (signal_number == SIGABRT) { signal_name = "SIGABRT"; }
		if (signal_number == SIGBUS)  { signal_name = "SIGBUS";  }
		if (signal_number == SIGFPE)  { signal_name = "SIGFPE";  }
		if (signal_number == SIGILL)  { signal_name = "SIGILL";  }
		if (signal_number == SIGINT)  { signal_name = "SIGINT";  }
		if (signal_number == SIGSEGV) { signal_name = "SIGSEGV"; }
		if (signal_number == SIGTERM) { signal_name = "SIGTERM"; }

		// --------------------------------------------------------------------
		/* There are few things that are safe to do in a signal handler,
		   but writing to stderr is one of them.
		   So we first print out what happened to stderr so we're sure that gets out,
		   then we do the unsafe things, like logging the stack trace.
		*/

		if (g_colorlogtostderr && s_terminal_has_color) {
			write_to_stderr(terminal_reset());
			write_to_stderr(terminal_bold());
			write_to_stderr(terminal_light_red());
		}
		write_to_stderr("\n");
		write_to_stderr("Loguru caught a signal: ");
		write_to_stderr(signal_name);
		write_to_stderr("\n");
		if (g_colorlogtostderr && s_terminal_has_color) {
			write_to_stderr(terminal_reset());
		}

		// --------------------------------------------------------------------

		if (s_signal_options.unsafe_signal_handler) {
			// --------------------------------------------------------------------
			/* Now we do unsafe things. This can for example lead to deadlocks if
			   the signal was triggered from the system's memory management functions
			   and the code below tries to do allocations.
			*/

			flush();
			char preamble_buff[LOGURU_PREAMBLE_WIDTH];
			print_preamble(preamble_buff, sizeof(preamble_buff), Verbosity_FATAL, "", 0);
			auto message = Message{Verbosity_FATAL, "", 0, preamble_buff, "", "Signal: ", signal_name};
			try {
				log_message(1, message, false, false);
			} catch (...) {
				// This can happed due to s_fatal_handler.
				write_to_stderr("Exception caught and ignored by Loguru signal handler.\n");
			}
			flush();

			// --------------------------------------------------------------------
		}

		call_default_signal_handler(signal_number);
	}

	void install_signal_handlers(const SignalOptions& signal_options)
	{
		s_signal_options = signal_options;

		struct sigaction sig_action;
		memset(&sig_action, 0, sizeof(sig_action));
		sigemptyset(&sig_action.sa_mask);
		sig_action.sa_flags |= SA_SIGINFO;
		sig_action.sa_sigaction = &signal_handler;

		if (signal_options.sigabrt) {
			CHECK_F(sigaction(SIGABRT, &sig_action, NULL) != -1, "Failed to install handler for SIGABRT");
		}
		if (signal_options.sigbus) {
			CHECK_F(sigaction(SIGBUS, &sig_action, NULL) != -1, "Failed to install handler for SIGBUS");
		}
		if (signal_options.sigfpe) {
			CHECK_F(sigaction(SIGFPE, &sig_action, NULL) != -1, "Failed to install handler for SIGFPE");
		}
		if (signal_options.sigill) {
			CHECK_F(sigaction(SIGILL, &sig_action, NULL) != -1, "Failed to install handler for SIGILL");
		}
		if (signal_options.sigint) {
			CHECK_F(sigaction(SIGINT, &sig_action, NULL) != -1, "Failed to install handler for SIGINT");
		}
		if (signal_options.sigsegv) {
			CHECK_F(sigaction(SIGSEGV, &sig_action, NULL) != -1, "Failed to install handler for SIGSEGV");
		}
		if (signal_options.sigterm) {
			CHECK_F(sigaction(SIGTERM, &sig_action, NULL) != -1, "Failed to install handler for SIGTERM");
		}
	}
} // namespace loguru

#endif // _WIN32


#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

LOGURU_ANONYMOUS_NAMESPACE_END

#endif // LOGURU_IMPLEMENTATION




#endif  //__LOGURU_HPP__