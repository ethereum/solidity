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
/** @file crypto.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Crypto test functions.
 */

#include <random>
#include <secp256k1/secp256k1.h>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/Log.h>
#include <libethereum/Transaction.h>
#include <boost/test/unit_test.hpp>
#include <libdevcrypto/EC.h>
#include <libdevcrypto/SHA3MAC.h>
#include "TestHelperCrypto.h"

using namespace std;
using namespace dev;
using namespace dev::crypto;
using namespace CryptoPP;

BOOST_AUTO_TEST_SUITE(devcrypto)

BOOST_AUTO_TEST_CASE(common_encrypt_decrypt)
{
	string message("Now is the time for all good persons to come to the aide of humanity.");
	bytes m = asBytes(message);
	bytesConstRef bcr(&m);

	KeyPair k = KeyPair::create();
	bytes cipher;
	encrypt(k.pub(), bcr, cipher);
	BOOST_REQUIRE(cipher != asBytes(message) && cipher.size() > 0);
	
	bytes plain;
	decrypt(k.sec(), bytesConstRef(&cipher), plain);
	
	BOOST_REQUIRE(asString(plain) == message);
	BOOST_REQUIRE(plain == asBytes(message));
}

BOOST_AUTO_TEST_CASE(cryptopp_vs_secp256k1)
{
	ECIES<ECP>::Decryptor d(pp::PRNG, pp::secp256k1Curve);
	ECIES<ECP>::Encryptor e(d.GetKey());
	
	Secret s;
	pp::exportPrivateKey(d.GetKey(), s);
	
	Public p;
	pp::exportPublicKey(e.GetKey(), p);
	
	BOOST_REQUIRE(dev::toAddress(s) == right160(dev::sha3(p.ref())));
	
	Secret previous = s;
	for (auto i = 0; i < 30; i++)
	{
		ECIES<ECP>::Decryptor d(pp::PRNG, pp::secp256k1Curve);
		ECIES<ECP>::Encryptor e(d.GetKey());
		
		Secret s;
		pp::exportPrivateKey(d.GetKey(), s);
		BOOST_REQUIRE(s != previous);
		
		Public p;
		pp::exportPublicKey(e.GetKey(), p);

		BOOST_REQUIRE(dev::toAddress(s) == right160(dev::sha3(p.ref())));
	}
}

BOOST_AUTO_TEST_CASE(cryptopp_ecdsa_sipaseckp256k1)
{
	// cryptopp integer encoding
	Integer nHex("f2ee15ea639b73fa3db9b34a245bdfa015c260c598b211bf05a1ecc4b3e3b4f2H");
	Integer nB(fromHex("f2ee15ea639b73fa3db9b34a245bdfa015c260c598b211bf05a1ecc4b3e3b4f2").data(), 32);
	BOOST_REQUIRE(nHex == nB);
	
	bytes sbytes(fromHex("0x01"));
	Secret secret(sha3(sbytes)); // 5fe7f977e71dba2ea1a68e21057beebb9be2ac30c6410aa38d4f3fbe41dcffd2
	KeyPair key(secret);
	
	bytes m(fromHex("0x01"));
	int tests = 5;
	while (m[0]++ && tests--)
	{
		h256 hm(sha3(m));
		Integer hInt(hm.asBytes().data(), 32);
		h256 k(hm ^ key.sec());
		Integer kInt(k.asBytes().data(), 32);
		
		// raw sign w/cryptopp (doesn't pass through cryptopp hash filter)
		ECDSA<ECP, SHA3_256>::Signer signer;
		pp::initializeSigner(key.sec(), signer);
		Integer r, s;
		signer.RawSign(kInt, hInt, r, s);

		// verify cryptopp raw-signature w/cryptopp
		ECDSA<ECP, SHA3_256>::Verifier verifier;
		pp::initializeVerifier(key.pub(), verifier);
		Signature sigppraw;
		r.Encode(sigppraw.data(), 32);
		s.Encode(sigppraw.data()+32, 32);
		BOOST_REQUIRE(verifier.VerifyMessage(m.data(), m.size(), sigppraw.data(), 64));
		BOOST_REQUIRE(crypto::verify(key.pub(), sigppraw, bytesConstRef(&m)));
		BOOST_REQUIRE(dev::verify(key.pub(), sigppraw, hm));
		BOOST_CHECK(dev::recover(sigppraw, hm) == key.pub());
		
		// sign with sec256lib, verify with cryptopp
		Signature seclibsig(dev::sign(key.sec(), hm));
		BOOST_REQUIRE(verifier.VerifyMessage(m.data(), m.size(), seclibsig.data(), 64));
		BOOST_REQUIRE(crypto::verify(key.pub(), seclibsig, bytesConstRef(&m)));
		BOOST_REQUIRE(dev::verify(key.pub(), seclibsig, hm));
		BOOST_CHECK(dev::recover(seclibsig, hm) == key.pub());

		// sign with cryptopp (w/hash filter?), verify with cryptopp
		bytes sigppb(signer.MaxSignatureLength());
		size_t ssz = signer.SignMessage(pp::PRNG, m.data(), m.size(), sigppb.data());
		Signature sigpp;
		memcpy(sigpp.data(), sigppb.data(), 64);
		BOOST_REQUIRE(verifier.VerifyMessage(m.data(), m.size(), sigppb.data(), ssz));
		BOOST_REQUIRE(crypto::verify(key.pub(), sigpp, bytesConstRef(&m)));
		BOOST_REQUIRE(dev::verify(key.pub(), sigpp, hm));
		BOOST_CHECK(dev::recover(sigpp, hm) == key.pub());

		// sign with cryptopp and stringsource hash filter
		string sigstr;
		StringSource ssrc(asString(m), true, new SignerFilter(pp::PRNG, signer, new StringSink(sigstr)));
		FixedHash<sizeof(Signature)> retsig((byte const*)sigstr.data(), Signature::ConstructFromPointer);
		BOOST_REQUIRE(verifier.VerifyMessage(m.data(), m.size(), retsig.data(), 64));
		BOOST_REQUIRE(crypto::verify(key.pub(), retsig, bytesConstRef(&m)));
		BOOST_REQUIRE(dev::verify(key.pub(), retsig, hm));
		BOOST_CHECK(dev::recover(retsig, hm) == key.pub());
		
		/// verification w/sec256lib
		// requires public key and sig in standard format
		byte encpub[65] = {0x04};
		memcpy(&encpub[1], key.pub().data(), 64);
		byte dersig[72];
		
		// verify sec256lib sig w/sec256lib
		size_t cssz = DSAConvertSignatureFormat(dersig, 72, DSA_DER, seclibsig.data(), 64, DSA_P1363);
		BOOST_CHECK(cssz <= 72);
		BOOST_REQUIRE(1 == secp256k1_ecdsa_verify(hm.data(), sizeof(hm), dersig, cssz, encpub, 65));
		
		// verify cryptopp-raw sig w/sec256lib
		cssz = DSAConvertSignatureFormat(dersig, 72, DSA_DER, sigppraw.data(), 64, DSA_P1363);
		BOOST_CHECK(cssz <= 72);
		BOOST_REQUIRE(1 == secp256k1_ecdsa_verify(hm.data(), sizeof(hm), dersig, cssz, encpub, 65));

		// verify cryptopp sig w/sec256lib
		cssz = DSAConvertSignatureFormat(dersig, 72, DSA_DER, sigppb.data(), 64, DSA_P1363);
		BOOST_CHECK(cssz <= 72);
		BOOST_REQUIRE(1 == secp256k1_ecdsa_verify(hm.data(), sizeof(hm), dersig, cssz, encpub, 65));
	}
}

BOOST_AUTO_TEST_CASE(cryptopp_public_export_import)
{
	ECIES<ECP>::Decryptor d(pp::PRNG, pp::secp256k1Curve);
	ECIES<ECP>::Encryptor e(d.GetKey());

	Secret s;
	pp::exportPrivateKey(d.GetKey(), s);
	Public p;
	pp::exportPublicKey(e.GetKey(), p);
	Address addr = right160(dev::sha3(p.ref()));
	BOOST_REQUIRE(toAddress(s) == addr);
	
	KeyPair l(s);
	BOOST_REQUIRE(l.address() == addr);
}

BOOST_AUTO_TEST_CASE(ecies_eckeypair)
{
	KeyPair k = KeyPair::create();

	string message("Now is the time for all good persons to come to the aide of humanity.");
	string original = message;
	
	bytes b = asBytes(message);
	encrypt(k.pub(), b);
	BOOST_REQUIRE(b != asBytes(original));

	decrypt(k.sec(), b);
	BOOST_REQUIRE(b == asBytes(original));
}

BOOST_AUTO_TEST_CASE(ecdhe_aes128_ctr_sha3mac)
{
	// New connections require new ECDH keypairs
	// Every new connection requires a new EC keypair
	// Every new trust requires a new EC keypair
	// All connections should share seed for PRF (or PRNG) for nonces
	
	
}

BOOST_AUTO_TEST_CASE(cryptopp_ecies_message)
{
	cnote << "Testing cryptopp_ecies_message...";

	string const message("Now is the time for all good persons to come to the aide of humanity.");

	ECIES<ECP>::Decryptor localDecryptor(pp::PRNG, pp::secp256k1Curve);
	SavePrivateKey(localDecryptor.GetPrivateKey());
	
	ECIES<ECP>::Encryptor localEncryptor(localDecryptor);
	SavePublicKey(localEncryptor.GetPublicKey());

	ECIES<ECP>::Decryptor futureDecryptor;
	LoadPrivateKey(futureDecryptor.AccessPrivateKey());
	futureDecryptor.GetPrivateKey().ThrowIfInvalid(pp::PRNG, 3);
	
	ECIES<ECP>::Encryptor futureEncryptor;
	LoadPublicKey(futureEncryptor.AccessPublicKey());
	futureEncryptor.GetPublicKey().ThrowIfInvalid(pp::PRNG, 3);

	// encrypt/decrypt with local
	string cipherLocal;
	StringSource ss1 (message, true, new PK_EncryptorFilter(pp::PRNG, localEncryptor, new StringSink(cipherLocal) ) );
	string plainLocal;
	StringSource ss2 (cipherLocal, true, new PK_DecryptorFilter(pp::PRNG, localDecryptor, new StringSink(plainLocal) ) );

	// encrypt/decrypt with future
	string cipherFuture;
	StringSource ss3 (message, true, new PK_EncryptorFilter(pp::PRNG, futureEncryptor, new StringSink(cipherFuture) ) );
	string plainFuture;
	StringSource ss4 (cipherFuture, true, new PK_DecryptorFilter(pp::PRNG, futureDecryptor, new StringSink(plainFuture) ) );
	
	// decrypt local w/future
	string plainFutureFromLocal;
	StringSource ss5 (cipherLocal, true, new PK_DecryptorFilter(pp::PRNG, futureDecryptor, new StringSink(plainFutureFromLocal) ) );
	
	// decrypt future w/local
	string plainLocalFromFuture;
	StringSource ss6 (cipherFuture, true, new PK_DecryptorFilter(pp::PRNG, localDecryptor, new StringSink(plainLocalFromFuture) ) );
	
	
	BOOST_REQUIRE(plainLocal == message);
	BOOST_REQUIRE(plainFuture == plainLocal);
	BOOST_REQUIRE(plainFutureFromLocal == plainLocal);
	BOOST_REQUIRE(plainLocalFromFuture == plainLocal);
}

BOOST_AUTO_TEST_CASE(cryptopp_aes128_ctr)
{
	const int aesKeyLen = 16;
	BOOST_REQUIRE(sizeof(char) == sizeof(byte));
	
	// generate test key
	AutoSeededRandomPool rng;
	SecByteBlock key(0x00, aesKeyLen);
	rng.GenerateBlock(key, key.size());
	
	// cryptopp uses IV as nonce/counter which is same as using nonce w/0 ctr
	byte ctr[AES::BLOCKSIZE];
	rng.GenerateBlock(ctr, sizeof(ctr));
	
	string text = "Now is the time for all good persons to come to the aide of humanity.";
	// c++11 ftw
	unsigned char const* in = (unsigned char*)&text[0];
	unsigned char* out = (unsigned char*)&text[0];
	string original = text;
	
	string cipherCopy;
	try
	{
		CTR_Mode<AES>::Encryption e;
		e.SetKeyWithIV(key, key.size(), ctr);
		e.ProcessData(out, in, text.size());
		BOOST_REQUIRE(text != original);
		cipherCopy = text;
	}
	catch(CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
	}
	
	try
	{
		CTR_Mode< AES >::Decryption d;
		d.SetKeyWithIV(key, key.size(), ctr);
		d.ProcessData(out, in, text.size());
		BOOST_REQUIRE(text == original);
	}
	catch(CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
	}
	
	
	// reencrypt ciphertext...
	try
	{
		BOOST_REQUIRE(cipherCopy != text);
		in = (unsigned char*)&cipherCopy[0];
		out = (unsigned char*)&cipherCopy[0];
		
		CTR_Mode<AES>::Encryption e;
		e.SetKeyWithIV(key, key.size(), ctr);
		e.ProcessData(out, in, text.size());
		
		// yep, ctr mode.
		BOOST_REQUIRE(cipherCopy == original);
	}
	catch(CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
	}
	
}

BOOST_AUTO_TEST_CASE(cryptopp_aes128_cbc)
{
	const int aesKeyLen = 16;
	BOOST_REQUIRE(sizeof(char) == sizeof(byte));
	
	AutoSeededRandomPool rng;
	SecByteBlock key(0x00, aesKeyLen);
	rng.GenerateBlock(key, key.size());
	
	// Generate random IV
	byte iv[AES::BLOCKSIZE];
	rng.GenerateBlock(iv, AES::BLOCKSIZE);
	
	string string128("AAAAAAAAAAAAAAAA");
	string plainOriginal = string128;
	
	CryptoPP::CBC_Mode<Rijndael>::Encryption cbcEncryption(key, key.size(), iv);
	cbcEncryption.ProcessData((byte*)&string128[0], (byte*)&string128[0], string128.size());
	BOOST_REQUIRE(string128 != plainOriginal);
	
	CBC_Mode<Rijndael>::Decryption cbcDecryption(key, key.size(), iv);
	cbcDecryption.ProcessData((byte*)&string128[0], (byte*)&string128[0], string128.size());
	BOOST_REQUIRE(plainOriginal == string128);
	
	
	// plaintext whose size isn't divisible by block size must use stream filter for padding
	string string192("AAAAAAAAAAAAAAAABBBBBBBB");
	plainOriginal = string192;

	string cipher;
	StreamTransformationFilter* aesStream = new StreamTransformationFilter(cbcEncryption, new StringSink(cipher));
	StringSource source(string192, true, aesStream);
	BOOST_REQUIRE(cipher.size() == 32);

	cbcDecryption.ProcessData((byte*)&cipher[0], (byte*)&string192[0], cipher.size());
	BOOST_REQUIRE(string192 == plainOriginal);
}

BOOST_AUTO_TEST_CASE(eth_keypairs)
{
	cnote << "Testing Crypto...";
	secp256k1_start();

	KeyPair p(Secret(fromHex("3ecb44df2159c26e0f995712d4f39b6f6e499b40749b1cf1246c37f9516cb6a4")));
	BOOST_REQUIRE(p.pub() == Public(fromHex("97466f2b32bc3bb76d4741ae51cd1d8578b48d3f1e68da206d47321aec267ce78549b514e4453d74ef11b0cd5e4e4c364effddac8b51bcfc8de80682f952896f")));
	BOOST_REQUIRE(p.address() == Address(fromHex("8a40bfaa73256b60764c1bf40675a99083efb075")));
	{
		eth::Transaction t;
		t.nonce = 0;
		t.receiveAddress = h160(fromHex("944400f4b88ac9589a0f17ed4671da26bddb668b"));
		t.value = 1000;
		auto rlp = t.rlp(false);
		cnote << RLP(rlp);
		cnote << toHex(rlp);
		cnote << t.sha3(false);
		t.sign(p.secret());
		rlp = t.rlp(true);
		cnote << RLP(rlp);
		cnote << toHex(rlp);
		cnote << t.sha3(true);
		BOOST_REQUIRE(t.sender() == p.address());
	}

} 
 

int cryptoTest()
{
	cnote << "Testing Crypto...";
	secp256k1_start();

	KeyPair p(Secret(fromHex("3ecb44df2159c26e0f995712d4f39b6f6e499b40749b1cf1246c37f9516cb6a4")));
	BOOST_REQUIRE(p.pub() == Public(fromHex("97466f2b32bc3bb76d4741ae51cd1d8578b48d3f1e68da206d47321aec267ce78549b514e4453d74ef11b0cd5e4e4c364effddac8b51bcfc8de80682f952896f")));
	BOOST_REQUIRE(p.address() == Address(fromHex("8a40bfaa73256b60764c1bf40675a99083efb075")));
	{
		eth::Transaction t;
		t.nonce = 0;
		t.receiveAddress = h160(fromHex("944400f4b88ac9589a0f17ed4671da26bddb668b"));
		t.value = 1000;
		auto rlp = t.rlp(false);
		cnote << RLP(rlp);
		cnote << toHex(rlp);
		cnote << t.sha3(false);
		t.sign(p.secret());
		rlp = t.rlp(true);
		cnote << RLP(rlp);
		cnote << toHex(rlp);
		cnote << t.sha3(true);
		BOOST_REQUIRE(t.sender() == p.address());
	}


#if 0
	// Test transaction.
	bytes tx = fromHex("88005401010101010101010101010101010101010101011f0de0b6b3a76400001ce8d4a5100080181c373130a009ba1f10285d4e659568bfcfec85067855c5a3c150100815dad4ef98fd37cf0593828c89db94bd6c64e210a32ef8956eaa81ea9307194996a3b879441f5d");
	cout << "TX: " << RLP(tx) << endl;

	Transaction t2(tx);
	cout << "SENDER: " << hex << t2.sender() << dec << endl;

	secp256k1_start();

	Transaction t;
	t.nonce = 0;
	t.value = 1;			// 1 wei.
	t.receiveAddress = toAddress(sha3("123"));

	bytes sig64 = toBigEndian(t.vrs.r) + toBigEndian(t.vrs.s);
	cout << "SIG: " << sig64.size() << " " << toHex(sig64) << " " << t.vrs.v << endl;

	auto msg = t.rlp(false);
	cout << "TX w/o SIG: " << RLP(msg) << endl;
	cout << "RLP(TX w/o SIG): " << toHex(t.rlpString(false)) << endl;
	std::string hmsg = sha3(t.rlpString(false), false);
	cout << "SHA256(RLP(TX w/o SIG)): 0x" << toHex(hmsg) << endl;

	bytes privkey = sha3Bytes("123");

	{
		bytes pubkey(65);
		int pubkeylen = 65;

		int ret = secp256k1_ecdsa_seckey_verify(privkey.data());
		cout << "SEC: " << dec << ret << " " << toHex(privkey) << endl;

		ret = secp256k1_ecdsa_pubkey_create(pubkey.data(), &pubkeylen, privkey.data(), 1);
		pubkey.resize(pubkeylen);
		int good = secp256k1_ecdsa_pubkey_verify(pubkey.data(), (int)pubkey.size());
		cout << "PUB: " << dec << ret << " " << pubkeylen << " " << toHex(pubkey) << (good ? " GOOD" : " BAD") << endl;
	}

	// Test roundtrip...
	{
		bytes sig(64);
		u256 nonce = 0;
		int v = 0;
		cout << toHex(hmsg) << endl;
		cout << toHex(privkey) << endl;
		cout << hex << nonce << dec << endl;
		int ret = secp256k1_ecdsa_sign_compact((byte const*)hmsg.data(), (int)hmsg.size(), sig.data(), privkey.data(), (byte const*)&nonce, &v);
		cout << "MYSIG: " << dec << ret << " " << sig.size() << " " << toHex(sig) << " " << v << endl;

		bytes pubkey(65);
		int pubkeylen = 65;
		ret = secp256k1_ecdsa_recover_compact((byte const*)hmsg.data(), (int)hmsg.size(), (byte const*)sig.data(), pubkey.data(), &pubkeylen, 0, v);
		pubkey.resize(pubkeylen);
		cout << "MYREC: " << dec << ret << " " << pubkeylen << " " << toHex(pubkey) << endl;
	}

	{
		bytes pubkey(65);
		int pubkeylen = 65;
		int ret = secp256k1_ecdsa_recover_compact((byte const*)hmsg.data(), (int)hmsg.size(), (byte const*)sig64.data(), pubkey.data(), &pubkeylen, 0, (int)t.vrs.v - 27);
		pubkey.resize(pubkeylen);
		cout << "RECPUB: " << dec << ret << " " << pubkeylen << " " << toHex(pubkey) << endl;
		cout << "SENDER: " << hex << toAddress(dev::sha3(bytesConstRef(&pubkey).cropped(1))) << dec << endl;
	}
#endif
	return 0;
}

BOOST_AUTO_TEST_SUITE_END()

