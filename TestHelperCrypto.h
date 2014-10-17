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
/** @file TestHelperCrypto.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 */

#pragma once

//#include <ostream>

#pragma warning(push)
#pragma warning(disable:4100 4244)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic ignored "-Wextra"
#include <osrng.h>
#include <eccrypto.h>		// secp256k1
#include <oids.h>		// ec domain
#include <ecp.h>			// ec prime field
#include <files.h>		// cryptopp buffer
#include <aes.h>
#include <modes.h>		// aes modes
#pragma warning(pop)
#pragma GCC diagnostic pop

using namespace std;
using namespace CryptoPP;

void SavePrivateKey(const PrivateKey& key, const string& file = "ecies.private.key")
{
	FileSink sink(file.c_str());
	key.Save(sink);
}

void SavePublicKey(const PublicKey& key, const string& file = "ecies.public.key")
{
	FileSink sink(file.c_str());
	key.Save(sink);
}

void LoadPrivateKey(PrivateKey& key, const string& file = "ecies.private.key")
{
	FileSource source(file.c_str(), true);
	key.Load(source);
}

void LoadPublicKey(PublicKey& key, const string& file = "ecies.public.key")
{
	FileSource source(file.c_str(), true);
	key.Load(source);
}
