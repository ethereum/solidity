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

#include "Common.h"

namespace dev
{

/// The null output stream. Used when logging is disabled.
class NullOutputStream
{
public:
	template <class T> NullOutputStream& operator<<(T const&) { return *this; }
};
/// The default logging channels. Each has an associated verbosity and three-letter prefix (name() ).
/// Channels should inherit from LogChannel and define name() and verbosity.
struct LogChannel { static const char* name(); static const int verbosity = 1; static const bool debug = true; };
struct WarnChannel: public LogChannel { static const char* name(); static const int verbosity = 0; static const bool debug = false; };


/// A "hacky" way to make sure the next statement get executed only once without the good old
/// do { } while(0) macro. We need such a thing due to the dangling else problem and the need
/// for the logging macros to end with the stream object and not a closing brace '}'
#define DEV_STATEMENT_ONCE() for (bool i_eth_once_ = true; i_eth_once_; i_eth_once_ = false)
// Kill all logs when when NLOG is defined.
#define clog(X) nlog(X)
#define cwarn clog(dev::WarnChannel)
// Null stream-like objects.
#define nlog(X) DEV_STATEMENT_ONCE() if (true) {} else dev::NullOutputStream()
}
