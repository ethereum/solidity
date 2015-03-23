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
/** @file block.cpp
 * @author Christoph Jentzsch <cj@ethdev.com>
 * @date 2015
 * block test functions.
 */

#include <libdevcrypto/FileSystem.h>
#include <libethereum/CanonBlockChain.h>
#include "TestHelper.h"

using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;

namespace dev {  namespace test {

bytes createBlockRLPFromFields(mObject& _tObj);
void overwriteBlockHeader(BlockInfo& _current_BlockHeader, mObject& _blObj);
BlockInfo constructBlock(mObject& _o);
void updatePoW(BlockInfo& _bi);
void writeBlockHeaderToJson(mObject& _o, const BlockInfo& _bi);
RLPStream createFullBlockFromHeader(const BlockInfo& _bi, const bytes& _txs = RLPEmptyList, const bytes& _uncles = RLPEmptyList);

void doBlockchainTests(json_spirit::mValue& _v, bool _fillin)
{
	for (auto& i: _v.get_obj())
	{
		cerr << i.first << endl;
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("genesisBlockHeader"));
		BlockInfo biGenesisBlock = constructBlock(o["genesisBlockHeader"].get_obj());

		BOOST_REQUIRE(o.count("pre"));
		ImportTest importer(o["pre"].get_obj());
		State state(biGenesisBlock.coinbaseAddress, OverlayDB(), BaseState::Empty);
		importer.importState(o["pre"].get_obj(), state);
		o["pre"] = fillJsonWithState(state);
		state.commit();

		if (_fillin)
			biGenesisBlock.stateRoot = state.rootHash();
		else
			BOOST_CHECK_MESSAGE(biGenesisBlock.stateRoot == state.rootHash(), "root hash does not match");

		if (_fillin)
		{
			// find new valid nonce
			updatePoW(biGenesisBlock);

			//update genesis block in json file
			writeBlockHeaderToJson(o["genesisBlockHeader"].get_obj(), biGenesisBlock);
		}

		// create new "genesis" block
		RLPStream rlpGenesisBlock = createFullBlockFromHeader(biGenesisBlock);
		biGenesisBlock.verifyInternals(&rlpGenesisBlock.out());
		o["genesisRLP"] = "0x" + toHex(rlpGenesisBlock.out());

		// construct blockchain
		BlockChain bc(rlpGenesisBlock.out(), string(), true);

		if (_fillin)
		{
			BOOST_REQUIRE(o.count("blocks"));
			mArray blArray;
			vector<BlockInfo> vBiBlocks;
			vBiBlocks.push_back(biGenesisBlock);
			for (auto const& bl: o["blocks"].get_array())
			{
				mObject blObj = bl.get_obj();

				// get txs
				TransactionQueue txs;
				ZeroGasPricer gp;
				BOOST_REQUIRE(blObj.count("transactions"));
				for (auto const& txObj: blObj["transactions"].get_array())
				{
					mObject tx = txObj.get_obj();
					importer.importTransaction(tx);
					if (!txs.attemptImport(importer.m_transaction.rlp()))
						cnote << "failed importing transaction\n";
				}

				// write uncle list
				BlockQueue uncleBlockQueue;
				mArray aUncleList;
				vector<BlockInfo> vBiUncles;
				mObject uncleHeaderObj_pre;

				for (auto const& uHObj: blObj["uncleHeaders"].get_array())
				{
					mObject uncleHeaderObj = uHObj.get_obj();
					if (uncleHeaderObj.count("sameAsPreviousSibling"))
					{
						writeBlockHeaderToJson(uncleHeaderObj_pre, vBiUncles[vBiUncles.size()-1]);
						aUncleList.push_back(uncleHeaderObj_pre);
						vBiUncles.push_back(vBiUncles[vBiUncles.size()-1]);
						continue;
					}

					if (uncleHeaderObj.count("sameAsBlock"))
					{
						writeBlockHeaderToJson(uncleHeaderObj_pre, vBiBlocks[(size_t)toInt(uncleHeaderObj["sameAsBlock"])]);
						aUncleList.push_back(uncleHeaderObj_pre);
						vBiUncles.push_back(vBiBlocks[(size_t)toInt(uncleHeaderObj["sameAsBlock"])]);
						continue;
					}

					BlockInfo uncleBlockFromFields = constructBlock(uncleHeaderObj);

					// make uncle header valid
					uncleBlockFromFields.timestamp = (u256)time(0);
					if (vBiBlocks.size() > 2)
					{
						if (uncleBlockFromFields.number - 1 < vBiBlocks.size())
							uncleBlockFromFields.populateFromParent(vBiBlocks[(size_t)uncleBlockFromFields.number - 1]);
						else
							uncleBlockFromFields.populateFromParent(vBiBlocks[vBiBlocks.size() - 2]);
					}
					else
						continue;

					updatePoW(uncleBlockFromFields);
					writeBlockHeaderToJson(uncleHeaderObj, uncleBlockFromFields);

					aUncleList.push_back(uncleHeaderObj);
					vBiUncles.push_back(uncleBlockFromFields);

					cnote << "import uncle in blockQueue";
					RLPStream uncle = createFullBlockFromHeader(uncleBlockFromFields);
					uncleBlockQueue.import(&uncle.out(), bc);

					uncleHeaderObj_pre = uncleHeaderObj;
				}

				blObj["uncleHeaders"] = aUncleList;
				bc.sync(uncleBlockQueue, state.db(), 4);
				state.commitToMine(bc);

				try
				{
					state.sync(bc);
					state.sync(bc, txs, gp);
					state.commitToMine(bc);
					MineInfo info;
					for (info.completed = false; !info.completed; info = state.mine()) {}
					state.completeMine();
				}
				catch (Exception const& _e)
				{
					cnote << "state sync or mining did throw an exception: " << diagnostic_information(_e);
					return;
				}
				catch (std::exception const& _e)
				{
					cnote << "state sync or mining did throw an exception: " << _e.what();
					return;
				}

				blObj["rlp"] = "0x" + toHex(state.blockData());

				// write valid txs
				mArray txArray;
				Transactions txList;
				for (auto const& txi: txs.transactions())
				{
					Transaction tx(txi.second, CheckSignature::Sender);
					txList.push_back(tx);
					mObject txObject;
					txObject["nonce"] = toString(tx.nonce());
					txObject["data"] = "0x" + toHex(tx.data());
					txObject["gasLimit"] = toString(tx.gas());
					txObject["gasPrice"] = toString(tx.gasPrice());
					txObject["r"] = "0x" + toString(tx.signature().r);
					txObject["s"] = "0x" + toString(tx.signature().s);
					txObject["v"] = to_string(tx.signature().v + 27);
					txObject["to"] = tx.isCreation() ? "" : toString(tx.receiveAddress());
					txObject["value"] = toString(tx.value());

					txArray.push_back(txObject);
				}

				blObj["transactions"] = txArray;

				BlockInfo current_BlockHeader = state.info();

				if (blObj.count("blockHeader"))
					overwriteBlockHeader(current_BlockHeader, blObj);

				// write block header
				mObject oBlockHeader;
				writeBlockHeaderToJson(oBlockHeader, current_BlockHeader);
				blObj["blockHeader"] = oBlockHeader;
				vBiBlocks.push_back(current_BlockHeader);

				// compare blocks from state and from rlp
				RLPStream txStream;
				txStream.appendList(txList.size());
				for (unsigned i = 0; i < txList.size(); ++i)
				{
					RLPStream txrlp;
					txList[i].streamRLP(txrlp);
					txStream.appendRaw(txrlp.out());
				}

				RLPStream uncleStream;
				uncleStream.appendList(vBiUncles.size());
				for (unsigned i = 0; i < vBiUncles.size(); ++i)
				{
					RLPStream uncleRlp;
					vBiUncles[i].streamRLP(uncleRlp, WithNonce);
					uncleStream.appendRaw(uncleRlp.out());
				}

				RLPStream block2 = createFullBlockFromHeader(current_BlockHeader, txStream.out(), uncleStream.out());

				blObj["rlp"] = "0x" + toHex(block2.out());

				if (sha3(RLP(state.blockData())[0].data()) != sha3(RLP(block2.out())[0].data()))
					cnote << "block header mismatch\n";

				if (sha3(RLP(state.blockData())[1].data()) != sha3(RLP(block2.out())[1].data()))
					cnote << "txs mismatch\n";

				if (sha3(RLP(state.blockData())[2].data()) != sha3(RLP(block2.out())[2].data()))
					cnote << "uncle list mismatch\n" << RLP(state.blockData())[2].data() << "\n" << RLP(block2.out())[2].data();
				try
				{
					state.sync(bc);
					bc.import(block2.out(), state.db());
					state.sync(bc);
					state.commit();
				}
				// if exception is thrown, RLP is invalid and no blockHeader, Transaction list, or Uncle list should be given
				catch (...)
				{
					cnote << "block is invalid!\n";
					blObj.erase(blObj.find("blockHeader"));
					blObj.erase(blObj.find("uncleHeaders"));
					blObj.erase(blObj.find("transactions"));
				}
				blArray.push_back(blObj);
			}
			o["blocks"] = blArray;
			o["postState"] = fillJsonWithState(state);
		}

		else
		{
			for (auto const& bl: o["blocks"].get_array())
			{
				mObject blObj = bl.get_obj();
				bytes blockRLP;
				try
				{
					state.sync(bc);
					blockRLP = importByteArray(blObj["rlp"].get_str());
					bc.import(blockRLP, state.db());
					state.sync(bc);
				}
				// if exception is thrown, RLP is invalid and no blockHeader, Transaction list, or Uncle list should be given
				catch (Exception const& _e)
				{
					cnote << "state sync or block import did throw an exception: " << diagnostic_information(_e);
					BOOST_CHECK(blObj.count("blockHeader") == 0);
					BOOST_CHECK(blObj.count("transactions") == 0);
					BOOST_CHECK(blObj.count("uncleHeaders") == 0);
					continue;
				}
				catch (std::exception const& _e)
				{
					cnote << "state sync or block import did throw an exception: " << _e.what();
					BOOST_CHECK(blObj.count("blockHeader") == 0);
					BOOST_CHECK(blObj.count("transactions") == 0);
					BOOST_CHECK(blObj.count("uncleHeaders") == 0);
					continue;
				}
				catch(...)
				{
					cnote << "state sync or block import did throw an exception\n";
					BOOST_CHECK(blObj.count("blockHeader") == 0);
					BOOST_CHECK(blObj.count("transactions") == 0);
					BOOST_CHECK(blObj.count("uncleHeaders") == 0);
					continue;
				}

				BOOST_REQUIRE(blObj.count("blockHeader"));

				mObject tObj = blObj["blockHeader"].get_obj();
				BlockInfo blockHeaderFromFields;
				const bytes c_rlpBytesBlockHeader = createBlockRLPFromFields(tObj);
				const RLP c_blockHeaderRLP(c_rlpBytesBlockHeader);
				blockHeaderFromFields.populateFromHeader(c_blockHeaderRLP, IgnoreNonce);

				BlockInfo blockFromRlp = bc.info();

				//Check the fields restored from RLP to original fields
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.headerHash(WithNonce) == blockFromRlp.headerHash(WithNonce), "hash in given RLP not matching the block hash!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.parentHash == blockFromRlp.parentHash, "parentHash in given RLP not matching the block parentHash!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.sha3Uncles == blockFromRlp.sha3Uncles, "sha3Uncles in given RLP not matching the block sha3Uncles!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.coinbaseAddress == blockFromRlp.coinbaseAddress,"coinbaseAddress in given RLP not matching the block coinbaseAddress!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.stateRoot == blockFromRlp.stateRoot, "stateRoot in given RLP not matching the block stateRoot!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.transactionsRoot == blockFromRlp.transactionsRoot, "transactionsRoot in given RLP not matching the block transactionsRoot!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.receiptsRoot == blockFromRlp.receiptsRoot, "receiptsRoot in given RLP not matching the block receiptsRoot!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.logBloom == blockFromRlp.logBloom, "logBloom in given RLP not matching the block logBloom!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.difficulty == blockFromRlp.difficulty, "difficulty in given RLP not matching the block difficulty!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.number == blockFromRlp.number, "number in given RLP not matching the block number!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.gasLimit == blockFromRlp.gasLimit,"gasLimit in given RLP not matching the block gasLimit!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.gasUsed == blockFromRlp.gasUsed, "gasUsed in given RLP not matching the block gasUsed!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.timestamp == blockFromRlp.timestamp, "timestamp in given RLP not matching the block timestamp!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.extraData == blockFromRlp.extraData, "extraData in given RLP not matching the block extraData!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.mixHash == blockFromRlp.mixHash, "mixHash in given RLP not matching the block mixHash!");
				BOOST_CHECK_MESSAGE(blockHeaderFromFields.nonce == blockFromRlp.nonce, "nonce in given RLP not matching the block nonce!");

				BOOST_CHECK_MESSAGE(blockHeaderFromFields == blockFromRlp, "However, blockHeaderFromFields != blockFromRlp!");

				//Check transaction list

				Transactions txsFromField;

				for (auto const& txObj: blObj["transactions"].get_array())
				{
					mObject tx = txObj.get_obj();

					BOOST_REQUIRE(tx.count("nonce"));
					BOOST_REQUIRE(tx.count("gasPrice"));
					BOOST_REQUIRE(tx.count("gasLimit"));
					BOOST_REQUIRE(tx.count("to"));
					BOOST_REQUIRE(tx.count("value"));
					BOOST_REQUIRE(tx.count("v"));
					BOOST_REQUIRE(tx.count("r"));
					BOOST_REQUIRE(tx.count("s"));
					BOOST_REQUIRE(tx.count("data"));

					try
					{
						Transaction t(createRLPStreamFromTransactionFields(tx).out(), CheckSignature::Sender);
						txsFromField.push_back(t);
					}
					catch (Exception const& _e)
					{
						BOOST_ERROR("Failed transaction constructor with Exception: " << diagnostic_information(_e));
					}
					catch (exception const& _e)
					{
						cnote << _e.what();
					}
				}

				Transactions txsFromRlp;
				RLP root(blockRLP);
				for (auto const& tr: root[1])
				{
					Transaction tx(tr.data(), CheckSignature::Sender);
					txsFromRlp.push_back(tx);
				}

				BOOST_CHECK_MESSAGE(txsFromRlp.size() == txsFromField.size(), "transaction list size does not match");

				for (size_t i = 0; i < txsFromField.size(); ++i)
				{
					BOOST_CHECK_MESSAGE(txsFromField[i].data() == txsFromRlp[i].data(), "transaction data in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].gas() == txsFromRlp[i].gas(), "transaction gasLimit in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].gasPrice() == txsFromRlp[i].gasPrice(), "transaction gasPrice in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].nonce() == txsFromRlp[i].nonce(), "transaction nonce in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].signature().r == txsFromRlp[i].signature().r, "transaction r in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].signature().s == txsFromRlp[i].signature().s, "transaction s in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].signature().v == txsFromRlp[i].signature().v, "transaction v in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].receiveAddress() == txsFromRlp[i].receiveAddress(), "transaction receiveAddress in rlp and in field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].value() == txsFromRlp[i].value(), "transaction receiveAddress in rlp and in field do not match");

					BOOST_CHECK_MESSAGE(txsFromField[i] == txsFromRlp[i], "transactions from  rlp and transaction from field do not match");
					BOOST_CHECK_MESSAGE(txsFromField[i].rlp() == txsFromRlp[i].rlp(), "transactions rlp do not match");

				}

				// check uncle list

				// uncles from uncle list field
				vector<BlockInfo> uBlHsFromField;
				if (blObj["uncleHeaders"].type() != json_spirit::null_type)
					for (auto const& uBlHeaderObj: blObj["uncleHeaders"].get_array())
					{
						mObject uBlH = uBlHeaderObj.get_obj();
						cout << "uBlH.size(): " << uBlH.size() << endl;
						BOOST_REQUIRE(uBlH.size() == 16);
						bytes uncleRLP = createBlockRLPFromFields(uBlH);
						const RLP c_uRLP(uncleRLP);
						BlockInfo uncleBlockHeader;
						try
						{
							uncleBlockHeader.populateFromHeader(c_uRLP);
						}
						catch(...)
						{
							BOOST_ERROR("invalid uncle header");
						}
						uBlHsFromField.push_back(uncleBlockHeader);
					}

				// uncles from block RLP
				vector<BlockInfo> uBlHsFromRlp;
				for	(auto const& uRLP: root[2])
				{
					BlockInfo uBl;
					uBl.populateFromHeader(uRLP);
					uBlHsFromRlp.push_back(uBl);
				}

				BOOST_REQUIRE_EQUAL(uBlHsFromField.size(), uBlHsFromRlp.size());

				for (size_t i = 0; i < uBlHsFromField.size(); ++i)
					BOOST_CHECK_MESSAGE(uBlHsFromField[i] == uBlHsFromRlp[i], "block header in rlp and in field do not match");
			}
		}
	}
}

// helping functions

bytes createBlockRLPFromFields(mObject& _tObj)
{
	RLPStream rlpStream;
	rlpStream.appendList(_tObj.count("hash") > 0 ? (_tObj.size() - 1) : _tObj.size());

	if (_tObj.count("parentHash"))
		rlpStream << importByteArray(_tObj["parentHash"].get_str());

	if (_tObj.count("uncleHash"))
		rlpStream << importByteArray(_tObj["uncleHash"].get_str());

	if (_tObj.count("coinbase"))
		rlpStream << importByteArray(_tObj["coinbase"].get_str());

	if (_tObj.count("stateRoot"))
		rlpStream << importByteArray(_tObj["stateRoot"].get_str());

	if (_tObj.count("transactionsTrie"))
		rlpStream << importByteArray(_tObj["transactionsTrie"].get_str());

	if (_tObj.count("receiptTrie"))
		rlpStream << importByteArray(_tObj["receiptTrie"].get_str());

	if (_tObj.count("bloom"))
		rlpStream << importByteArray(_tObj["bloom"].get_str());

	if (_tObj.count("difficulty"))
		rlpStream << bigint(_tObj["difficulty"].get_str());

	if (_tObj.count("number"))
		rlpStream << bigint(_tObj["number"].get_str());

	if (_tObj.count("gasLimit"))
		rlpStream << bigint(_tObj["gasLimit"].get_str());

	if (_tObj.count("gasUsed"))
		rlpStream << bigint(_tObj["gasUsed"].get_str());

	if (_tObj.count("timestamp"))
		rlpStream << bigint(_tObj["timestamp"].get_str());

	if (_tObj.count("extraData"))
		rlpStream << fromHex(_tObj["extraData"].get_str());

	if (_tObj.count("mixHash"))
		rlpStream << importByteArray(_tObj["mixHash"].get_str());

	if (_tObj.count("nonce"))
		rlpStream << importByteArray(_tObj["nonce"].get_str());

	return rlpStream.out();
}

void overwriteBlockHeader(BlockInfo& _current_BlockHeader, mObject& _blObj)
{
	if (_blObj["blockHeader"].get_obj().size() != 14)
	{

		BlockInfo tmp = _current_BlockHeader;

		if (_blObj["blockHeader"].get_obj().count("parentHash"))
			tmp.parentHash = h256(_blObj["blockHeader"].get_obj()["parentHash"].get_str());

		if (_blObj["blockHeader"].get_obj().count("uncleHash"))
			tmp.sha3Uncles = h256(_blObj["blockHeader"].get_obj()["uncleHash"].get_str());

		if (_blObj["blockHeader"].get_obj().count("coinbase"))
			tmp.coinbaseAddress = Address(_blObj["blockHeader"].get_obj()["coinbase"].get_str());

		if (_blObj["blockHeader"].get_obj().count("stateRoot"))
			tmp.stateRoot = h256(_blObj["blockHeader"].get_obj()["stateRoot"].get_str());

		if (_blObj["blockHeader"].get_obj().count("transactionsTrie"))
			tmp.transactionsRoot = h256(_blObj["blockHeader"].get_obj()["transactionsTrie"].get_str());

		if (_blObj["blockHeader"].get_obj().count("receiptTrie"))
			tmp.receiptsRoot = h256(_blObj["blockHeader"].get_obj()["receiptTrie"].get_str());

		if (_blObj["blockHeader"].get_obj().count("bloom"))
			tmp.logBloom = LogBloom(_blObj["blockHeader"].get_obj()["bloom"].get_str());

		if (_blObj["blockHeader"].get_obj().count("difficulty"))
			tmp.difficulty = toInt(_blObj["blockHeader"].get_obj()["difficulty"]);

		if (_blObj["blockHeader"].get_obj().count("number"))
			tmp.number = toInt(_blObj["blockHeader"].get_obj()["number"]);

		if (_blObj["blockHeader"].get_obj().count("gasLimit"))
			tmp.gasLimit = toInt(_blObj["blockHeader"].get_obj()["gasLimit"]);

		if (_blObj["blockHeader"].get_obj().count("gasUsed"))
			tmp.gasUsed = toInt(_blObj["blockHeader"].get_obj()["gasUsed"]);

		if (_blObj["blockHeader"].get_obj().count("timestamp"))
			tmp.timestamp = toInt(_blObj["blockHeader"].get_obj()["timestamp"]);

		if (_blObj["blockHeader"].get_obj().count("extraData"))
			tmp.extraData = importByteArray(_blObj["blockHeader"].get_obj()["extraData"].get_str());

		if (_blObj["blockHeader"].get_obj().count("mixHash"))
			tmp.mixHash = h256(_blObj["blockHeader"].get_obj()["mixHash"].get_str());

		// find new valid nonce

		if (tmp != _current_BlockHeader)
		{
			_current_BlockHeader = tmp;

			ProofOfWork pow;
			std::pair<MineInfo, Ethash::Proof> ret;
			while (!ProofOfWork::verify(_current_BlockHeader))
			{
				ret = pow.mine(_current_BlockHeader, 1000, true, true);
				Ethash::assignResult(ret.second, _current_BlockHeader);
			}
		}
	}
	else
	{
		// take the blockheader as is
		const bytes c_blockRLP = createBlockRLPFromFields(_blObj["blockHeader"].get_obj());
		const RLP c_bRLP(c_blockRLP);
		_current_BlockHeader.populateFromHeader(c_bRLP, IgnoreNonce);
	}
}

BlockInfo constructBlock(mObject& _o)
{

	BlockInfo ret;
	try
	{
		// construct genesis block
		const bytes c_blockRLP = createBlockRLPFromFields(_o);
		const RLP c_bRLP(c_blockRLP);
		ret.populateFromHeader(c_bRLP, IgnoreNonce);
	}
	catch (Exception const& _e)
	{
		cnote << "block population did throw an exception: " << diagnostic_information(_e);
		BOOST_ERROR("Failed block population with Exception: " << _e.what());
	}
	catch (std::exception const& _e)
	{
		BOOST_ERROR("Failed block population with Exception: " << _e.what());
	}
	catch(...)
	{
		BOOST_ERROR("block population did throw an unknown exception\n");
	}
	return ret;
}

void updatePoW(BlockInfo& _bi)
{
	ProofOfWork pow;
	std::pair<MineInfo, Ethash::Proof> ret;
	while (!ProofOfWork::verify(_bi))
	{
		ret = pow.mine(_bi, 10000, true, true);
		Ethash::assignResult(ret.second, _bi);
	}
	_bi.hash = _bi.headerHash(WithNonce);
}

void writeBlockHeaderToJson(mObject& _o, const BlockInfo& _bi)
{
	_o["parentHash"] = toString(_bi.parentHash);
	_o["uncleHash"] = toString(_bi.sha3Uncles);
	_o["coinbase"] = toString(_bi.coinbaseAddress);
	_o["stateRoot"] = toString(_bi.stateRoot);
	_o["transactionsTrie"] = toString(_bi.transactionsRoot);
	_o["receiptTrie"] = toString(_bi.receiptsRoot);
	_o["bloom"] = toString(_bi.logBloom);
	_o["difficulty"] = toString(_bi.difficulty);
	_o["number"] = toString(_bi.number);
	_o["gasLimit"] = toString(_bi.gasLimit);
	_o["gasUsed"] = toString(_bi.gasUsed);
	_o["timestamp"] = toString(_bi.timestamp);
	_o["extraData"] ="0x" + toHex(_bi.extraData);
	_o["mixHash"] = toString(_bi.mixHash);
	_o["nonce"] = toString(_bi.nonce);
	_o["hash"] = toString(_bi.hash);
}

RLPStream createFullBlockFromHeader(const BlockInfo& _bi,const bytes& _txs, const bytes& _uncles )
{
	RLPStream rlpStream;
	_bi.streamRLP(rlpStream, WithNonce);

	RLPStream ret(3);
	ret.appendRaw(rlpStream.out());
	ret.appendRaw(_txs);
	ret.appendRaw(_uncles);

	return ret;
}
} }// Namespace Close


BOOST_AUTO_TEST_SUITE(BlockChainTests)

BOOST_AUTO_TEST_CASE(bcInvalidRLPTest)
{
	dev::test::executeTests("bcInvalidRLPTest", "/BlockTests", dev::test::doBlockchainTests);
}

BOOST_AUTO_TEST_CASE(bcJS_API_Test)
{
	dev::test::executeTests("bcJS_API_Test", "/BlockTests", dev::test::doBlockchainTests);
}

BOOST_AUTO_TEST_CASE(bcValidBlockTest)
{
	dev::test::executeTests("bcValidBlockTest", "/BlockTests", dev::test::doBlockchainTests);
}

BOOST_AUTO_TEST_CASE(bcInvalidHeaderTest)
{
	dev::test::executeTests("bcInvalidHeaderTest", "/BlockTests", dev::test::doBlockchainTests);
}

BOOST_AUTO_TEST_CASE(bcUncleTest)
{
	dev::test::executeTests("bcUncleTest", "/BlockTests", dev::test::doBlockchainTests);
}

BOOST_AUTO_TEST_CASE(userDefinedFile)
{
	dev::test::userDefinedTest("--singletest", dev::test::doBlockchainTests);
}

BOOST_AUTO_TEST_SUITE_END()

