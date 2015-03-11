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
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2015
 * RLPx test functions.
 */

#include <random>
#include <secp256k1/secp256k1.h>
#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/Log.h>
#include <libethereum/Transaction.h>
#include <boost/test/unit_test.hpp>
#include <libdevcrypto/SHA3.h>
#include <libdevcrypto/ECDHE.h>
#include <libdevcrypto/CryptoPP.h>
#include <libp2p/RLPxHandshake.h>

using namespace std;
using namespace dev;
using namespace dev::crypto;
using namespace CryptoPP;

BOOST_AUTO_TEST_SUITE(rlpx)

static Secp256k1 s_secp256k1;
static CryptoPP::AutoSeededRandomPool s_rng;
static CryptoPP::OID s_curveOID(CryptoPP::ASN1::secp256k1());
static CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> s_params(s_curveOID);
static CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>::EllipticCurve s_curve(s_params.GetCurve());

BOOST_AUTO_TEST_CASE(test_secrets_cpp_vectors)
{
	KeyPair init(Secret(sha3("initiator")));
	KeyPair initR(Secret(sha3("initiator-random")));
	h256 initNonce(sha3("initiator-nonce"));
	
	KeyPair recv(Secret(sha3("remote-recv")));
	KeyPair recvR(Secret(sha3("remote-recv-random")));
	h256 recvNonce(sha3("remote-recv-nonce"));
	
	bytes authCipher(fromHex(""));
	bytes ackCipher(fromHex(""));
	
	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_frameEnc;
	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_frameDec;
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption m_macEnc;
	CryptoPP::SHA3_256 m_egressMac;
	CryptoPP::SHA3_256 m_ingressMac;
	
	// when originated is true, agreement is with init secrets
	// when originated is true, remoteNonce = recvNonce
	// when originated is true, nonce = initNonce
	bool originated = true;
	auto remoteNonce = recvNonce;
	auto nonce = initNonce;
	bytes keyMaterialBytes(64);
	bytesRef keyMaterial(&keyMaterialBytes);
	
	// shared-secret = sha3(ecdhe-shared-secret || sha3(nonce || initiator-nonce))
	Secret ephemeralShared;
	s_secp256k1.agree(initR.sec(), recvR.pub(), ephemeralShared);
	Secret expected(fromHex("20d82c1092f351dc217bd66fa183e801234af14ead40423b6ee25112201c6e5a"));
	BOOST_REQUIRE(expected == ephemeralShared);
	
	ephemeralShared.ref().copyTo(keyMaterial.cropped(0, h256::size));
	h512 nonceMaterial;
	h256 const& leftNonce = originated ? remoteNonce : nonce;
	h256 const& rightNonce = originated ? nonce : remoteNonce;
	leftNonce.ref().copyTo(nonceMaterial.ref().cropped(0, h256::size));
	rightNonce.ref().copyTo(nonceMaterial.ref().cropped(h256::size, h256::size));
	auto outRef(keyMaterial.cropped(h256::size, h256::size));
	sha3(nonceMaterial.ref(), outRef); // output h(nonces)
	
	// test that keyMaterial = ecdhe-shared-secret || sha3(nonce || initiator-nonce)
	{
		BOOST_REQUIRE(ephemeralShared == *(Secret*)keyMaterialBytes.data());
		
		SHA3_256 ctx;
		ctx.Update(leftNonce.data(), h256::size);
		ctx.Update(rightNonce.data(), h256::size);
		bytes expected(32);
		ctx.Final(expected.data());
		bytes given(32);
		outRef.copyTo(&given);
		BOOST_REQUIRE(expected == given);
	}
	bytes preImage(keyMaterialBytes);
	
	// shared-secret <- sha3(ecdhe-shared-secret || sha3(nonce || initiator-nonce))
	// keyMaterial = ecdhe-shared-secret || shared-secret
	sha3(keyMaterial, outRef);
	bytes sharedSecret(32);
	outRef.copyTo(&sharedSecret);
	BOOST_REQUIRE(sharedSecret == fromHex("b65319ce56e00f3be75c4d0da92b5957d5583ca25eeeedac8e29b6dfc8b1ddf7"));
	
	// test that keyMaterial = ecdhe-shared-secret || shared-secret
	{
		BOOST_REQUIRE(ephemeralShared == *(Secret*)keyMaterialBytes.data());
		
		SHA3_256 ctx;
		ctx.Update(preImage.data(), preImage.size());
		bytes expected(32);
		ctx.Final(expected.data());
		bytes test(32);
		outRef.copyTo(&test);
		BOOST_REQUIRE(expected == test);
	}
	
	// token: sha3(outRef)
	bytes token(32);
	sha3(outRef, bytesRef(&token));
	BOOST_REQUIRE(token == fromHex("db41fe0180f372983cf19fca7ee890f7fb5481079d44683d2c027be9e71bbca2"));
	
	// aes-secret = sha3(ecdhe-shared-secret || shared-secret)
	sha3(keyMaterial, outRef); // output aes-secret
	bytes aesSecret(32);
	outRef.copyTo(&aesSecret);
	BOOST_REQUIRE(aesSecret == fromHex("12347b4784bcb4e74b84637940482852fe25d78e328cf5c6f7a396bf96cc20bb"));
	m_frameEnc.SetKeyWithIV(outRef.data(), h128::size, h128().data());
	m_frameDec.SetKeyWithIV(outRef.data(), h128::size, h128().data());
	
	// mac-secret = sha3(ecdhe-shared-secret || aes-secret)
	sha3(keyMaterial, outRef); // output mac-secret
	bytes macSecret(32);
	outRef.copyTo(&macSecret);
	BOOST_REQUIRE(macSecret == fromHex("2ec149072353d54437422837c886b0538a9206e6c559f6b4a55f65a866867723"));
	m_macEnc.SetKey(outRef.data(), h128::size);
	
	// Initiator egress-mac: sha3(mac-secret^recipient-nonce || auth-sent-init)
	//           ingress-mac: sha3(mac-secret^initiator-nonce || auth-recvd-ack)
	// Recipient egress-mac: sha3(mac-secret^initiator-nonce || auth-sent-ack)
	//           ingress-mac: sha3(mac-secret^recipient-nonce || auth-recvd-init)
	
	(*(h256*)outRef.data() ^ remoteNonce).ref().copyTo(keyMaterial);
	bytes const& egressCipher = originated ? authCipher : ackCipher;
	keyMaterialBytes.resize(h256::size + egressCipher.size());
	keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
	bytesConstRef(&egressCipher).copyTo(keyMaterial.cropped(h256::size, egressCipher.size()));
	m_egressMac.Update(keyMaterial.data(), keyMaterial.size());
	
	{
		bytes egressMac;
		SHA3_256 h(m_egressMac);
		bytes digest(16);
		h.TruncatedFinal(digest.data(), 16);
		BOOST_REQUIRE(digest == fromHex("23e5e8efb6e3765ecae1fca9160b18df"));
	}
	
	// recover mac-secret by re-xoring remoteNonce
	(*(h256*)keyMaterial.data() ^ remoteNonce ^ nonce).ref().copyTo(keyMaterial);
	bytes const& ingressCipher = originated ? ackCipher : authCipher;
	keyMaterialBytes.resize(h256::size + ingressCipher.size());
	keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
	bytesConstRef(&ingressCipher).copyTo(keyMaterial.cropped(h256::size, ingressCipher.size()));
	m_ingressMac.Update(keyMaterial.data(), keyMaterial.size());
	
	{
		bytes ingressMac;
		SHA3_256 h(m_ingressMac);
		bytes digest(16);
		h.TruncatedFinal(digest.data(), 16);
		BOOST_REQUIRE(digest == fromHex("ceed64135852064cbdde86e7ea05e8f5"));
	}
}

BOOST_AUTO_TEST_CASE(test_secrets_from_go)
{
	KeyPair init(Secret(fromHex("0x5e173f6ac3c669587538e7727cf19b782a4f2fda07c1eaa662c593e5e85e3051")));
	KeyPair initR(Secret(fromHex("0x19c2185f4f40634926ebed3af09070ca9e029f2edd5fae6253074896205f5f6c")));
	h256 initNonce(fromHex("0xcd26fecb93657d1cd9e9eaf4f8be720b56dd1d39f190c4e1c6b7ec66f077bb11"));

	KeyPair recv(Secret(fromHex("0xc45f950382d542169ea207959ee0220ec1491755abe405cd7498d6b16adb6df8")));
	KeyPair recvR(Secret(fromHex("0xd25688cf0ab10afa1a0e2dba7853ed5f1e5bf1c631757ed4e103b593ff3f5620")));
	h256 recvNonce(fromHex("0xf37ec61d84cea03dcc5e8385db93248584e8af4b4d1c832d8c7453c0089687a7"));

	bytes authCipher(fromHex("0x04a0274c5951e32132e7f088c9bdfdc76c9d91f0dc6078e848f8e3361193dbdc43b94351ea3d89e4ff33ddcefbc80070498824857f499656c4f79bbd97b6c51a514251d69fd1785ef8764bd1d262a883f780964cce6a14ff206daf1206aa073a2d35ce2697ebf3514225bef186631b2fd2316a4b7bcdefec8d75a1025ba2c5404a34e7795e1dd4bc01c6113ece07b0df13b69d3ba654a36e35e69ff9d482d88d2f0228e7d96fe11dccbb465a1831c7d4ad3a026924b182fc2bdfe016a6944312021da5cc459713b13b86a686cf34d6fe6615020e4acf26bf0d5b7579ba813e7723eb95b3cef9942f01a58bd61baee7c9bdd438956b426a4ffe238e61746a8c93d5e10680617c82e48d706ac4953f5e1c4c4f7d013c87d34a06626f498f34576dc017fdd3d581e83cfd26cf125b6d2bda1f1d56"));
	bytes ackCipher(fromHex("0x049934a7b2d7f9af8fd9db941d9da281ac9381b5740e1f64f7092f3588d4f87f5ce55191a6653e5e80c1c5dd538169aa123e70dc6ffc5af1827e546c0e958e42dad355bcc1fcb9cdf2cf47ff524d2ad98cbf275e661bf4cf00960e74b5956b799771334f426df007350b46049adb21a6e78ab1408d5e6ccde6fb5e69f0f4c92bb9c725c02f99fa72b9cdc8dd53cff089e0e73317f61cc5abf6152513cb7d833f09d2851603919bf0fbe44d79a09245c6e8338eb502083dc84b846f2fee1cc310d2cc8b1b9334728f97220bb799376233e113"));
	
	bytes authPlainExpected(fromHex("0x884c36f7ae6b406637c1f61b2f57e1d2cab813d24c6559aaf843c3f48962f32f46662c066d39669b7b2e3ba14781477417600e7728399278b1b5d801a519aa570034fdb5419558137e0d44cd13d319afe5629eeccb47fd9dfe55cc6089426e46cc762dd8a0636e07a54b31169eba0c7a20a1ac1ef68596f1f283b5c676bae4064abfcce24799d09f67e392632d3ffdc12e3d6430dcb0ea19c318343ffa7aae74d4cd26fecb93657d1cd9e9eaf4f8be720b56dd1d39f190c4e1c6b7ec66f077bb1100"));
	bytes ackPlainExpected(fromHex("0x802b052f8b066640bba94a4fc39d63815c377fced6fcb84d27f791c9921ddf3e9bf0108e298f490812847109cbd778fae393e80323fd643209841a3b7f110397f37ec61d84cea03dcc5e8385db93248584e8af4b4d1c832d8c7453c0089687a700"));
	
	bytes authPlain = authCipher;
	BOOST_REQUIRE(s_secp256k1.decryptECIES(recv.sec(), authPlain));
	bytes ackPlain = ackCipher;
	BOOST_REQUIRE(s_secp256k1.decryptECIES(init.sec(), ackPlain));
	
	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_frameEnc;
	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption m_frameDec;
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption m_macEnc;
	CryptoPP::SHA3_256 m_egressMac;
	CryptoPP::SHA3_256 m_ingressMac;
	
	// when originated is true, agreement is with init secrets
	// when originated is true, remoteNonce = recvNonce
	// when originated is true, nonce = initNonce
	bool originated = true;
	auto remoteNonce = recvNonce;
	auto nonce = initNonce;
	bytes keyMaterialBytes(64);
	bytesRef keyMaterial(&keyMaterialBytes);
	
	// shared-secret = sha3(ecdhe-shared-secret || sha3(nonce || initiator-nonce))
	Secret ephemeralShared;
	s_secp256k1.agree(initR.sec(), recvR.pub(), ephemeralShared);
	Secret expected(fromHex("0xe3f407f83fc012470c26a93fdff534100f2c6f736439ce0ca90e9914f7d1c381"));
	BOOST_REQUIRE(expected == ephemeralShared);
	
	ephemeralShared.ref().copyTo(keyMaterial.cropped(0, h256::size));
	h512 nonceMaterial;
	h256 const& leftNonce = originated ? remoteNonce : nonce;
	h256 const& rightNonce = originated ? nonce : remoteNonce;
	leftNonce.ref().copyTo(nonceMaterial.ref().cropped(0, h256::size));
	rightNonce.ref().copyTo(nonceMaterial.ref().cropped(h256::size, h256::size));
	auto outRef(keyMaterial.cropped(h256::size, h256::size));
	sha3(nonceMaterial.ref(), outRef); // output h(nonces)
	
	// test that keyMaterial = ecdhe-shared-secret || sha3(nonce || initiator-nonce)
	{
		BOOST_REQUIRE(ephemeralShared == *(Secret*)keyMaterialBytes.data());
		
		SHA3_256 ctx;
		ctx.Update(leftNonce.data(), h256::size);
		ctx.Update(rightNonce.data(), h256::size);
		bytes expected(32);
		ctx.Final(expected.data());
		bytes given(32);
		outRef.copyTo(&given);
		BOOST_REQUIRE(expected == given);
	}
	bytes preImage(keyMaterialBytes);
	
	// shared-secret <- sha3(ecdhe-shared-secret || sha3(nonce || initiator-nonce))
	// keyMaterial = ecdhe-shared-secret || shared-secret
	sha3(keyMaterial, outRef);
	
	// test that keyMaterial = ecdhe-shared-secret || shared-secret
	{
		BOOST_REQUIRE(ephemeralShared == *(Secret*)keyMaterialBytes.data());
		
		SHA3_256 ctx;
		ctx.Update(preImage.data(), preImage.size());
		bytes expected(32);
		ctx.Final(expected.data());
		bytes test(32);
		outRef.copyTo(&test);
		BOOST_REQUIRE(expected == test);
	}
	
	// token: sha3(outRef)
	bytes token(32);
	sha3(outRef, bytesRef(&token));
	BOOST_REQUIRE(token == fromHex("0x3f9ec2592d1554852b1f54d228f042ed0a9310ea86d038dc2b401ba8cd7fdac4"));
	
	// aes-secret = sha3(ecdhe-shared-secret || shared-secret)
	sha3(keyMaterial, outRef); // output aes-secret
	bytes aesSecret(32);
	outRef.copyTo(&aesSecret);
	BOOST_REQUIRE(aesSecret == fromHex("0xc0458fa97a5230830e05f4f20b7c755c1d4e54b1ce5cf43260bb191eef4e418d"));
	m_frameEnc.SetKeyWithIV(outRef.data(), h128::size, h128().data());
	m_frameDec.SetKeyWithIV(outRef.data(), h128::size, h128().data());
	
	// mac-secret = sha3(ecdhe-shared-secret || aes-secret)
	sha3(keyMaterial, outRef); // output mac-secret
	bytes macSecret(32);
	outRef.copyTo(&macSecret);
	BOOST_REQUIRE(macSecret == fromHex("0x48c938884d5067a1598272fcddaa4b833cd5e7d92e8228c0ecdfabbe68aef7f1"));
	m_macEnc.SetKey(outRef.data(), h256::size);
	
	// Initiator egress-mac: sha3(mac-secret^recipient-nonce || auth-sent-init)
	//           ingress-mac: sha3(mac-secret^initiator-nonce || auth-recvd-ack)
	// Recipient egress-mac: sha3(mac-secret^initiator-nonce || auth-sent-ack)
	//           ingress-mac: sha3(mac-secret^recipient-nonce || auth-recvd-init)

	(*(h256*)outRef.data() ^ remoteNonce).ref().copyTo(keyMaterial);
	bytes const& egressCipher = originated ? authCipher : ackCipher;
	keyMaterialBytes.resize(h256::size + egressCipher.size());
	keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
	bytesConstRef(&egressCipher).copyTo(keyMaterial.cropped(h256::size, egressCipher.size()));
	m_egressMac.Update(keyMaterialBytes.data(), keyMaterialBytes.size());
	
	{
		bytes egressMac;
		SHA3_256 h(m_egressMac);
		bytes digest(32);
		h.Final(digest.data());
		BOOST_REQUIRE(digest == fromHex("0x09771e93b1a6109e97074cbe2d2b0cf3d3878efafe68f53c41bb60c0ec49097e"));
	}
	
	// recover mac-secret by re-xoring remoteNonce
	bytes recoverMacSecretTest(32);
	(*(h256*)keyMaterial.data() ^ remoteNonce).ref().copyTo(&recoverMacSecretTest);
	BOOST_REQUIRE(recoverMacSecretTest == macSecret);
	
	(*(h256*)keyMaterial.data() ^ remoteNonce ^ nonce).ref().copyTo(keyMaterial);
	bytes const& ingressCipher = originated ? ackCipher : authCipher;
	keyMaterialBytes.resize(h256::size + ingressCipher.size());
	keyMaterial.retarget(keyMaterialBytes.data(), keyMaterialBytes.size());
	bytesConstRef(&ingressCipher).copyTo(keyMaterial.cropped(h256::size, ingressCipher.size()));
	m_ingressMac.Update(keyMaterial.data(), keyMaterial.size());

	{
		bytes ingressMac;
		SHA3_256 h(m_ingressMac);
		bytes digest(32);
		h.Final(digest.data());
		BOOST_CHECK(digest == fromHex("0x75823d96e23136c89666ee025fb21a432be906512b3dd4a3049e898adb433847"));
	}
	
	bytes initHello(fromHex("6ef23fcf1cec7312df623f9ae701e63b550cdb8517fefd8dd398fc2acd1d935e6e0434a2b96769078477637347b7b01924fff9ff1c06df2f804df3b0402bbb9f87365b3c6856b45e1e2b6470986813c3816a71bff9d69dd297a5dbd935ab578f6e5d7e93e4506a44f307c332d95e8a4b102585fd8ef9fc9e3e055537a5cec2e9"));
	
	bytes recvHello(fromHex("6ef23fcf1cec7312df623f9ae701e63be36a1cdd1b19179146019984f3625d4a6e0434a2b96769050577657247b7b02bc6c314470eca7e3ef650b98c83e9d7dd4830b3f718ff562349aead2530a8d28a8484604f92e5fced2c6183f304344ab0e7c301a0c05559f4c25db65e36820b4b909a226171a60ac6cb7beea09376d6d8"));

	/// test macs of frame headers
	{
		SHA3_256 egressmac(m_egressMac);
		SHA3_256 prevDigest(egressmac);
		h128 prevDigestOut;
		prevDigest.TruncatedFinal(prevDigestOut.data(), h128::size);
		h128 encDigest;
		m_macEnc.ProcessData(encDigest.data(), prevDigestOut.data(), h128::size);
		encDigest ^= *(h128*)initHello.data();
		egressmac.Update(encDigest.data(), h128::size);
		egressmac.TruncatedFinal(encDigest.data(), h128::size);
		
		bytes provided(16);
		bytesConstRef(&initHello).cropped(16, 16).copyTo(bytesRef(&provided));
		BOOST_REQUIRE(*(h128*)encDigest.data() == *(h128*)provided.data());
	}
	
	{
		SHA3_256 ingressmac(m_ingressMac);
		SHA3_256 prevDigest(ingressmac);
		h128 prevDigestOut;
		prevDigest.TruncatedFinal(prevDigestOut.data(), h128::size);
		h128 encDigest;
		m_macEnc.ProcessData(encDigest.data(), prevDigestOut.data(), h128::size);
		encDigest ^= *(h128*)recvHello.data();
		ingressmac.Update(encDigest.data(), h128::size);
		ingressmac.TruncatedFinal(encDigest.data(), h128::size);
		
		bytes provided(16);
		bytesConstRef(&recvHello).cropped(16, 16).copyTo(bytesRef(&provided));
		BOOST_REQUIRE(*(h128*)encDigest.data() == *(h128*)provided.data());
	}
	
	// test decrypt of frame headers for recvHello
	bytes plaintext(16);
	m_frameDec.ProcessData(plaintext.data(), recvHello.data(), h128::size);
	
}

BOOST_AUTO_TEST_CASE(ecies_interop_test_primitives)
{
	CryptoPP::SHA256 sha256ctx;
	bytes emptyExpected(fromHex("0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
	bytes empty;
	sha256ctx.Update(empty.data(), 0);
	bytes emptyTestOut(32);
	sha256ctx.Final(emptyTestOut.data());
	BOOST_REQUIRE(emptyExpected == emptyTestOut);
	
	bytes hash1Expected(fromHex("0x8949b278bbafb8da1aaa18cb724175c5952280f74be5d29ab4b37d1b45c84b08"));
	bytes hash1input(fromHex("0x55a53b55afb12affff3c"));
	sha256ctx.Update(hash1input.data(), hash1input.size());
	bytes hash1Out(32);
	sha256ctx.Final(hash1Out.data());
	BOOST_REQUIRE(hash1Out == hash1Expected);
	
	h128 hmack(fromHex("0x07a4b6dfa06369a570f2dcba2f11a18f"));
	CryptoPP::HMAC<SHA256> hmacctx(hmack.data(), h128::size);
	bytes input(fromHex("0x4dcb92ed4fc67fe86832"));
	hmacctx.Update(input.data(), input.size());
	bytes hmacExpected(fromHex("0xc90b62b1a673b47df8e395e671a68bfa68070d6e2ef039598bb829398b89b9a9"));
	bytes hmacOut(hmacExpected.size());
	hmacctx.Final(hmacOut.data());
	BOOST_REQUIRE(hmacExpected == hmacOut);
	
	// go messageTag
	bytes tagSecret(fromHex("0xaf6623e52208c596e17c72cea6f1cb09"));
	bytes tagInput(fromHex("0x3461282bcedace970df2"));
	bytes tagExpected(fromHex("0xb3ce623bce08d5793677ba9441b22bb34d3e8a7de964206d26589df3e8eb5183"));
	CryptoPP::HMAC<SHA256> hmactagctx(tagSecret.data(), tagSecret.size());
	hmactagctx.Update(tagInput.data(), tagInput.size());
	h256 mac;
	hmactagctx.Final(mac.data());
	BOOST_REQUIRE(mac.asBytes() == tagExpected);
	
	Secret input1(fromHex("0x0de72f1223915fa8b8bf45dffef67aef8d89792d116eb61c9a1eb02c422a4663"));
	bytes expect1(fromHex("0x1d0c446f9899a3426f2b89a8cb75c14b"));
	bytes test1;
	test1 = s_secp256k1.eciesKDF(input1, bytes(), 16);
	BOOST_REQUIRE(test1 == expect1);
	
	Secret kdfInput2(fromHex("0x961c065873443014e0371f1ed656c586c6730bf927415757f389d92acf8268df"));
	bytes kdfExpect2(fromHex("0x4050c52e6d9c08755e5a818ac66fabe478b825b1836fd5efc4d44e40d04dabcc"));
	bytes kdfTest2;
	kdfTest2 = s_secp256k1.eciesKDF(kdfInput2, bytes(), 32);
	BOOST_REQUIRE(kdfTest2 == kdfExpect2);
	
	KeyPair k(Secret(fromHex("0x332143e9629eedff7d142d741f896258f5a1bfab54dab2121d3ec5000093d74b")));
	Public p(fromHex("0xf0d2b97981bd0d415a843b5dfe8ab77a30300daab3658c578f2340308a2da1a07f0821367332598b6aa4e180a41e92f4ebbae3518da847f0b1c0bbfe20bcf4e1"));
	Secret agreeExpected(fromHex("0xee1418607c2fcfb57fda40380e885a707f49000a5dda056d828b7d9bd1f29a08"));
	Secret agreeTest;
	s_secp256k1.agree(k.sec(), p, agreeTest);
	BOOST_REQUIRE(agreeExpected == agreeTest);
	
	KeyPair kmK(Secret(fromHex("0x57baf2c62005ddec64c357d96183ebc90bf9100583280e848aa31d683cad73cb")));
	bytes kmCipher(fromHex("0x04ff2c874d0a47917c84eea0b2a4141ca95233720b5c70f81a8415bae1dc7b746b61df7558811c1d6054333907333ef9bb0cc2fbf8b34abb9730d14e0140f4553f4b15d705120af46cf653a1dc5b95b312cf8444714f95a4f7a0425b67fc064d18f4d0a528761565ca02d97faffdac23de10"));
	bytes kmPlain = kmCipher;
	bytes kmExpected(asBytes("a"));
	BOOST_REQUIRE(s_secp256k1.decryptECIES(kmK.sec(), kmPlain));
	BOOST_REQUIRE(kmExpected == kmPlain);
	
	KeyPair kenc(Secret(fromHex("0x472413e97f1fd58d84e28a559479e6b6902d2e8a0cee672ef38a3a35d263886b")));
	Public penc(Public(fromHex("0x7a2aa2951282279dc1171549a7112b07c38c0d97c0fe2c0ae6c4588ba15be74a04efc4f7da443f6d61f68a9279bc82b73e0cc8d090048e9f87e838ae65dd8d4c")));
	BOOST_REQUIRE(penc == kenc.pub());
	
	bytes cipher1(fromHex("0x046f647e1bd8a5cd1446d31513bac233e18bdc28ec0e59d46de453137a72599533f1e97c98154343420d5f16e171e5107999a7c7f1a6e26f57bcb0d2280655d08fb148d36f1d4b28642d3bb4a136f0e33e3dd2e3cffe4b45a03fb7c5b5ea5e65617250fdc89e1a315563c20504b9d3a72555"));
	bytes plainTest1 = cipher1;
	bytes expectedPlain1 = asBytes("a");
	BOOST_REQUIRE(s_secp256k1.decryptECIES(kenc.sec(), plainTest1));
	BOOST_REQUIRE(plainTest1 == expectedPlain1);
	
	bytes cipher2(fromHex("0x0443c24d6ccef3ad095140760bb143078b3880557a06392f17c5e368502d79532bc18903d59ced4bbe858e870610ab0d5f8b7963dd5c9c4cf81128d10efd7c7aa80091563c273e996578403694673581829e25a865191bdc9954db14285b56eb0043b6288172e0d003c10f42fe413222e273d1d4340c38a2d8344d7aadcbc846ee"));
	bytes plainTest2 = cipher2;
	bytes expectedPlain2 = asBytes("aaaaaaaaaaaaaaaa");
	BOOST_REQUIRE(s_secp256k1.decryptECIES(kenc.sec(), plainTest2));
	BOOST_REQUIRE(plainTest2 == expectedPlain2);
	
	bytes cipher3(fromHex("0x04c4e40c86bb5324e017e598c6d48c19362ae527af8ab21b077284a4656c8735e62d73fb3d740acefbec30ca4c024739a1fcdff69ecaf03301eebf156eb5f17cca6f9d7a7e214a1f3f6e34d1ee0ec00ce0ef7d2b242fbfec0f276e17941f9f1bfbe26de10a15a6fac3cda039904ddd1d7e06e7b96b4878f61860e47f0b84c8ceb64f6a900ff23844f4359ae49b44154980a626d3c73226c19e"));
	bytes plainTest3 = cipher3;
	bytes expectedPlain3 = asBytes("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	BOOST_REQUIRE(s_secp256k1.decryptECIES(kenc.sec(), plainTest3));
	BOOST_REQUIRE(plainTest3 == expectedPlain3);
}

BOOST_AUTO_TEST_SUITE_END()

