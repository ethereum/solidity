/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Log.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * The logging subsystem.
 */

#pragma once

#include <ctime>
#include <chrono>
#include "vector_ref.h"
#include "Common.h"
#include "CommonIO.h"
#include "CommonData.h"
#include "FixedHash.h"
#include "Terminal.h"

namespace boost { namespace asio { namespace ip { template<class T>class basic_endpoint; class tcp; } } }

namespace dev
{

/// The null output stream. Used when logging is disabled.
class NullOutputStream
{
public:
	template <class T> NullOutputStream& operator<<(T const&) { return *this; }
};

/// A simple log-output function that prints log messages to stdout.
void simpleDebugOut(std::string const&, char const*);

/// The logging system's current verbosity.
extern int g_logVerbosity;

/// The current method that the logging system uses to output the log messages. Defaults to simpleDebugOut().
extern std::function<void(std::string const&, char const*)> g_logPost;

class LogOverrideAux
{
protected:
	LogOverrideAux(std::type_info const* _ch, bool _value);
	~LogOverrideAux();

private:
	std::type_info const* m_ch;
	static const int c_null = -1;
	int m_old;
};

template <class Channel>
class LogOverride: LogOverrideAux
{
public:
	LogOverride(bool _value): LogOverrideAux(&typeid(Channel), _value) {}
};

bool isChannelVisible(std::type_info const* _ch, bool _default);
template <class Channel> bool isChannelVisible() { return isChannelVisible(&typeid(Channel), Channel::verbosity <= g_logVerbosity); }

/// Temporary changes system's verbosity for specific function. Restores the old verbosity when function returns.
/// Not thread-safe, use with caution!
struct VerbosityHolder
{
	VerbosityHolder(int _temporaryValue, bool _force = false): oldLogVerbosity(g_logVerbosity) { if (g_logVerbosity >= 0 || _force) g_logVerbosity = _temporaryValue; }
	~VerbosityHolder() { g_logVerbosity = oldLogVerbosity; }
	int oldLogVerbosity;
};

#define ETH_THREAD_CONTEXT(name) for (std::pair<dev::ThreadContext, bool> __eth_thread_context(name, true); p.second; p.second = false)

class ThreadContext
{
public:
	ThreadContext(std::string const& _info) { push(_info); }
	~ThreadContext() { pop(); }

	static void push(std::string const& _n);
	static void pop();
	static std::string join(std::string const& _prior);
};

/// Set the current thread's log name.
void setThreadName(std::string const& _n);

/// Set the current thread's log name.
std::string getThreadName();

/// The default logging channels. Each has an associated verbosity and three-letter prefix (name() ).
/// Channels should inherit from LogChannel and define name() and verbosity.
struct LogChannel { static const char* name(); static const int verbosity = 1; static const bool debug = true; };
struct LeftChannel: public LogChannel { static const char* name(); };
struct RightChannel: public LogChannel { static const char* name(); };
struct WarnChannel: public LogChannel { static const char* name(); static const int verbosity = 0; static const bool debug = false; };
struct NoteChannel: public LogChannel { static const char* name(); static const bool debug = false; };
struct DebugChannel: public LogChannel { static const char* name(); static const int verbosity = 0; };
struct TraceChannel: public LogChannel { static const char* name(); static const int verbosity = 4; static const bool debug = true; };

enum class LogTag
{
	None,
	Url,
	Error,
	Special
};

class LogOutputStreamBase
{
public:
	LogOutputStreamBase(char const* _id, std::type_info const* _info, unsigned _v, bool _autospacing);

	void comment(std::string const& _t)
	{
		switch (m_logTag)
		{
		case LogTag::Url: m_sstr << EthNavyUnder; break;
		case LogTag::Error: m_sstr << EthRedBold; break;
		case LogTag::Special: m_sstr << EthWhiteBold; break;
		default:;
		}
		m_sstr << _t << EthReset;
		m_logTag = LogTag::None;
	}

	void append(unsigned long _t) { m_sstr << EthBlue << _t << EthReset; }
	void append(long _t) { m_sstr << EthBlue << _t << EthReset; }
	void append(unsigned int _t) { m_sstr << EthBlue << _t << EthReset; }
	void append(int _t) { m_sstr << EthBlue << _t << EthReset; }
	void append(bigint const& _t) { m_sstr << EthNavy << _t << EthReset; }
	void append(u256 const& _t) { m_sstr << EthNavy << _t << EthReset; }
	void append(u160 const& _t) { m_sstr << EthNavy << _t << EthReset; }
	void append(double _t) { m_sstr << EthBlue << _t << EthReset; }
	template <unsigned N> void append(FixedHash<N> const& _t) { m_sstr << EthTeal "#" << _t.abridged() << EthReset; }
	void append(h160 const& _t) { m_sstr << EthRed "@" << _t.abridged() << EthReset; }
	void append(h256 const& _t) { m_sstr << EthCyan "#" << _t.abridged() << EthReset; }
	void append(h512 const& _t) { m_sstr << EthTeal "##" << _t.abridged() << EthReset; }
	void append(std::string const& _t) { m_sstr << EthGreen "\"" + _t + "\"" EthReset; }
	void append(bytes const& _t) { m_sstr << EthYellow "%" << toHex(_t) << EthReset; }
	void append(bytesConstRef _t) { m_sstr << EthYellow "%" << toHex(_t) << EthReset; }
	void append(boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> const& _t);
	template <class T> void append(std::vector<T> const& _t)
	{
		m_sstr << EthWhite "[" EthReset;
		int n = 0;
		for (auto const& i: _t)
		{
			m_sstr << (n++ ? EthWhite ", " EthReset : "");
			append(i);
		}
		m_sstr << EthWhite "]" EthReset;
	}
	template <class T> void append(std::set<T> const& _t)
	{
		m_sstr << EthYellow "{" EthReset;
		int n = 0;
		for (auto const& i: _t)
		{
			m_sstr << (n++ ? EthYellow ", " EthReset : "");
			append(i);
		}
		m_sstr << EthYellow "}" EthReset;
	}
	template <class T, class U> void append(std::map<T, U> const& _t)
	{
		m_sstr << EthLime "{" EthReset;
		int n = 0;
		for (auto const& i: _t)
		{
			m_sstr << (n++ ? EthLime ", " EthReset : "");
			append(i.first);
			m_sstr << (n++ ? EthLime ": " EthReset : "");
			append(i.second);
		}
		m_sstr << EthLime "}" EthReset;
	}
	template <class T> void append(std::unordered_set<T> const& _t)
	{
		m_sstr << EthYellow "{" EthReset;
		int n = 0;
		for (auto const& i: _t)
		{
			m_sstr << (n++ ? EthYellow ", " EthReset : "");
			append(i);
		}
		m_sstr << EthYellow "}" EthReset;
	}
	template <class T, class U> void append(std::unordered_map<T, U> const& _t)
	{
		m_sstr << EthLime "{" EthReset;
		int n = 0;
		for (auto const& i: _t)
		{
			m_sstr << (n++ ? EthLime ", " EthReset : "");
			append(i.first);
			m_sstr << (n++ ? EthLime ": " EthReset : "");
			append(i.second);
		}
		m_sstr << EthLime "}" EthReset;
	}
	template <class T, class U> void append(std::pair<T, U> const& _t)
	{
		m_sstr << EthPurple "(" EthReset;
		append(_t.first);
		m_sstr << EthPurple ", " EthReset;
		append(_t.second);
		m_sstr << EthPurple ")" EthReset;
	}
	template <class T> void append(T const& _t)
	{
		m_sstr << toString(_t);
	}

protected:
	bool m_autospacing = false;
	unsigned m_verbosity = 0;
	std::stringstream m_sstr;	///< The accrued log entry.
	LogTag m_logTag = LogTag::None;
};

/// Logging class, iostream-like, that can be shifted to.
template <class Id, bool _AutoSpacing = true>
class LogOutputStream: LogOutputStreamBase
{
public:
	/// Construct a new object.
	/// If _term is true the the prefix info is terminated with a ']' character; if not it ends only with a '|' character.
	LogOutputStream(): LogOutputStreamBase(Id::name(), &typeid(Id), Id::verbosity, _AutoSpacing) {}

	/// Destructor. Posts the accrued log entry to the g_logPost function.
	~LogOutputStream() { if (Id::verbosity <= g_logVerbosity) g_logPost(m_sstr.str(), Id::name()); }

	LogOutputStream& operator<<(std::string const& _t) { if (Id::verbosity <= g_logVerbosity) { if (_AutoSpacing && m_sstr.str().size() && m_sstr.str().back() != ' ') m_sstr << " "; comment(_t); } return *this; }

	LogOutputStream& operator<<(LogTag _t) { m_logTag = _t; return *this; }

	/// Shift arbitrary data to the log. Spaces will be added between items as required.
	template <class T> LogOutputStream& operator<<(T const& _t) { if (Id::verbosity <= g_logVerbosity) { if (_AutoSpacing && m_sstr.str().size() && m_sstr.str().back() != ' ') m_sstr << " "; append(_t); } return *this; }
};

/// A "hacky" way to make sure the next statement get executed only once without the good old
/// do { } while(0) macro. We need such a thing due to the dangling else problem and the need
/// for the logging macros to end with the stream object and not a closing brace '}'
#define DEV_STATEMENT_ONCE() for (bool i_eth_once_ = true; i_eth_once_; i_eth_once_ = false)
// Kill all logs when when NLOG is defined.
#if NLOG
#define clog(X) nlog(X)
#define cslog(X) nslog(X)
#else
#if NDEBUG
#define clog(X) DEV_STATEMENT_ONCE() if (X::debug) {} else dev::LogOutputStream<X, true>()
#define cslog(X) DEV_STATEMENT_ONCE() if (X::debug) {} else dev::LogOutputStream<X, false>()
#else
#define clog(X) dev::LogOutputStream<X, true>()
#define cslog(X) dev::LogOutputStream<X, false>()
#endif
#endif

// Simple cout-like stream objects for accessing common log channels.
// Dirties the global namespace, but oh so convenient...
#define cdebug clog(dev::DebugChannel)
#define cnote clog(dev::NoteChannel)
#define cwarn clog(dev::WarnChannel)
#define ctrace clog(dev::TraceChannel)

// Null stream-like objects.
#define ndebug DEV_STATEMENT_ONCE() if (true) {} else dev::NullOutputStream()
#define nlog(X) DEV_STATEMENT_ONCE() if (true) {} else dev::NullOutputStream()
#define nslog(X) DEV_STATEMENT_ONCE() if (true) {} else dev::NullOutputStream()

}
