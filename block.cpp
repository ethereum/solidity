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

#include <libethereum/CanonBlockChain.h>
#include "TestHelper.h"

using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;

namespace dev {  namespace test {

bytes createBlockRLPFromFields(mObject& _tObj)
{
	RLPStream rlpStream;
	rlpStream.appendList(_tObj.size());

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
		rlpStream << importByteArray(_tObj["extraData"].get_str());

	if (_tObj.count("nonce"))
		rlpStream << importByteArray(_tObj["nonce"].get_str());

	return rlpStream.out();
}

void doBlockTests(json_spirit::mValue& _v, bool _fillin)
{
	for (auto& i: _v.get_obj())
	{
		cerr << i.first << endl;
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("genesisBlockHeader"));
		BlockInfo blockFromFields;
		try
		{
			// construct genesis block
			const bytes c_blockRLP = createBlockRLPFromFields(o["genesisBlockHeader"].get_obj());
			const RLP c_bRLP(c_blockRLP);
			blockFromFields.populateFromHeader(c_bRLP, false);
		}
		catch (Exception const& _e)
		{
			cnote << "block population did throw an exception: " << diagnostic_information(_e);
			BOOST_ERROR("Failed block population with Exception: " << _e.what());
			continue;
		}
		catch (std::exception const& _e)
		{
			BOOST_ERROR("Failed block population with Exception: " << _e.what());
			continue;
		}
		catch(...)
		{
			cnote << "block population did throw an unknown exception\n";
			continue;
		}

		BOOST_REQUIRE(o.count("pre"));

		ImportTest importer(o["pre"].get_obj());
		State state(Address(), OverlayDB(), BaseState::Empty);
		importer.importState(o["pre"].get_obj(), state);
		state.commit();

		if (_fillin)
			blockFromFields.stateRoot = state.rootHash();
		else
			BOOST_CHECK_MESSAGE(blockFromFields.stateRoot == state.rootHash(), "root hash does not match");

		if (_fillin)
		{
			// find new valid nonce
			ProofOfWork pow;
			MineInfo ret;
			while (!ProofOfWork::verify(blockFromFields.headerHash(WithoutNonce), blockFromFields.nonce, blockFromFields.difficulty))
				tie(ret, blockFromFields.nonce) = pow.mine(blockFromFields.headerHash(WithoutNonce), blockFromFields.difficulty, 1000, true, true);

			//update genesis block in json file
			o["genesisBlockHeader"].get_obj()["stateRoot"] = toString(blockFromFields.stateRoot);
			o["genesisBlockHeader"].get_obj()["nonce"] = toString(blockFromFields.nonce);
		}

		// create new "genesis" block
		RLPStream rlpStream;
		blockFromFields.streamRLP(rlpStream, WithNonce);

		RLPStream block(3);
		block.appendRaw(rlpStream.out());
		block.appendRaw(RLPEmptyList);
		block.appendRaw(RLPEmptyList);

		blockFromFields.verifyInternals(&block.out());

		// construct blockchain
		BlockChain bc(block.out(), string(), true);

		if (_fillin)
		{
			BOOST_REQUIRE(o.count("transactions"));

			TransactionQueue txs;

			for (auto const& txObj: o["transactions"].get_array())
			{
				mObject tx = txObj.get_obj();
				importer.importTransaction(tx);
				if (!txs.attemptImport(importer.m_transaction.rlp()))
					cnote << "failed importing transaction\n";
			}

			try
			{
				state.sync(bc);
				state.sync(bc,txs);
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

			// write valid txs
			mArray txArray;
			Transactions txList;
			for (auto const& txi: txs.transactions())
			{
				Transaction tx(txi.second, CheckSignature::Sender);
				txList.push_back(tx);
				mObject txObject;
				txObject["nonce"] = toString(tx.nonce());
				txObject["data"] = toHex(tx.data());
				txObject["gasLimit"] = toString(tx.gas());
				txObject["gasPrice"] = toString(tx.gasPrice());
				txObject["r"] = "0x" + toString(tx.signature().r);
				txObject["s"] = "0x" + toString(tx.signature().s);
				txObject["v"] = to_string(tx.signature().v + 27);
				txObject["to"] = toString(tx.receiveAddress());
				txObject["value"] = toString(tx.value());

				txArray.push_back(txObject);
			}

			o["transactions"] = txArray;
			o["rlp"] = "0x" + toHex(state.blockData());

			BlockInfo current_BlockHeader = state.info();

			// overwrite blockheader with (possible wrong) data from "blockheader" in filler;

			if (o.count("blockHeader"))
			{
				if (o["blockHeader"].get_obj().size() != 14)
				{

					BlockInfo tmp = current_BlockHeader;

					if (o["blockHeader"].get_obj().count("parentHash"))
						tmp.parentHash = h256(o["blockHeader"].get_obj()["parentHash"].get_str());

					if (o["blockHeader"].get_obj().count("uncleHash"))
						tmp.sha3Uncles = h256(o["blockHeader"].get_obj()["uncleHash"].get_str());

					if (o["blockHeader"].get_obj().count("coinbase"))
						tmp.coinbaseAddress = Address(o["blockHeader"].get_obj()["coinbase"].get_str());

					if (o["blockHeader"].get_obj().count("stateRoot"))
						tmp.stateRoot = h256(o["blockHeader"].get_obj()["stateRoot"].get_str());

					if (o["blockHeader"].get_obj().count("transactionsTrie"))
						tmp.transactionsRoot = h256(o["blockHeader"].get_obj()["transactionsTrie"].get_str());

					if (o["blockHeader"].get_obj().count("receiptTrie"))
						tmp.receiptsRoot = h256(o["blockHeader"].get_obj()["receiptTrie"].get_str());

					if (o["blockHeader"].get_obj().count("bloom"))
						tmp.logBloom = LogBloom(o["blockHeader"].get_obj()["bloom"].get_str());

					if (o["blockHeader"].get_obj().count("difficulty"))
						tmp.difficulty = toInt(o["blockHeader"].get_obj()["difficulty"]);

					if (o["blockHeader"].get_obj().count("number"))
						tmp.number = toInt(o["blockHeader"].get_obj()["number"]);

					if (o["blockHeader"].get_obj().count("gasLimit"))
						tmp.gasLimit = toInt(o["blockHeader"].get_obj()["gasLimit"]);

					if (o["blockHeader"].get_obj().count("gasUsed"))
						tmp.gasUsed = toInt(o["blockHeader"].get_obj()["gasUsed"]);

					if (o["blockHeader"].get_obj().count("timestamp"))
						tmp.timestamp = toInt(o["blockHeader"].get_obj()["timestamp"]);

					if (o["blockHeader"].get_obj().count("extraData"))
						tmp.extraData = importByteArray(o["blockHeader"].get_obj()["extraData"].get_str());

					// find new valid nonce

					if (tmp != current_BlockHeader)
					{
						current_BlockHeader = tmp;
						cout << "new header!\n";
						ProofOfWork pow;
						MineInfo ret;
						while (!ProofOfWork::verify(current_BlockHeader.headerHash(WithoutNonce), current_BlockHeader.nonce, current_BlockHeader.difficulty))
							tie(ret, current_BlockHeader.nonce) = pow.mine(current_BlockHeader.headerHash(WithoutNonce), current_BlockHeader.difficulty, 10000, true, true);
					}
				}
				else
				{
					// take the blockheader as is
					const bytes c_blockRLP = createBlockRLPFromFields(o["blockHeader"].get_obj());
					const RLP c_bRLP(c_blockRLP);
					current_BlockHeader.populateFromHeader(c_bRLP, false);
				}
			}

			// write block header

			mObject oBlockHeader;
			oBlockHeader["parentHash"] = toString(current_BlockHeader.parentHash);
			oBlockHeader["uncleHash"] = toString(current_BlockHeader.sha3Uncles);
			oBlockHeader["coinbase"] = toString(current_BlockHeader.coinbaseAddress);
			oBlockHeader["stateRoot"] = toString(current_BlockHeader.stateRoot);
			oBlockHeader["transactionsTrie"] = toString(current_BlockHeader.transactionsRoot);
			oBlockHeader["receiptTrie"] = toString(current_BlockHeader.receiptsRoot);
			oBlockHeader["bloom"] = toString(current_BlockHeader.logBloom);
			oBlockHeader["difficulty"] = toString(current_BlockHeader.difficulty);
			oBlockHeader["number"] = toString(current_BlockHeader.number);
			oBlockHeader["gasLimit"] = toString(current_BlockHeader.gasLimit);
			oBlockHeader["gasUsed"] = toString(current_BlockHeader.gasUsed);
			oBlockHeader["timestamp"] = toString(current_BlockHeader.timestamp);
			oBlockHeader["extraData"] = toHex(current_BlockHeader.extraData);
			oBlockHeader["nonce"] = toString(current_BlockHeader.nonce);

			o["blockHeader"] = oBlockHeader;

			// write uncle list
			mArray aUncleList; // as of now, our parent is always the genesis block, so we can not have uncles.
			o["uncleHeaders"] = aUncleList;

			//txs:

			RLPStream txStream;
			txStream.appendList(txList.size());
			for (unsigned i = 0; i < txList.size(); ++i)
			{
				RLPStream txrlp;
				txList[i].streamRLP(txrlp);
				txStream.appendRaw(txrlp.out());
			}

			RLPStream rlpStream2;
			current_BlockHeader.streamRLP(rlpStream2, WithNonce);

			RLPStream block2(3);
			block2.appendRaw(rlpStream2.out());
			block2.appendRaw(txStream.out());
			block2.appendRaw(RLPEmptyList);

			o["rlp"] = "0x" + toHex(block2.out());

			if (sha3(RLP(state.blockData())[0].data()) != sha3(RLP(block2.out())[0].data()))
				cnote << "block header mismatch\n";

			if (sha3(RLP(state.blockData())[1].data()) != sha3(RLP(block2.out())[1].data()))
				cnote << "txs mismatch\n";

			if (sha3(RLP(state.blockData())[2].data()) != sha3(RLP(block2.out())[2].data()))
				cnote << "uncle list mismatch\n";

			try
			{
				ImportTest importerTmp(o["pre"].get_obj());
				State stateTmp(Address(), OverlayDB(), BaseState::Empty);
				importerTmp.importState(o["pre"].get_obj(), stateTmp);
				stateTmp.commit();
				BlockChain bcTmp(block.out(), "/tmp/", true);
				stateTmp.sync(bcTmp);
				bc.import(block2.out(), stateTmp.db());
				stateTmp.sync(bcTmp);
			}
			// if exception is thrown, RLP is invalid and no blockHeader, Transaction list, or Uncle list should be given
			catch (...)
			{
				cnote << "block is invalid!\n";
				o.erase(o.find("blockHeader"));
				o.erase(o.find("uncleHeaders"));
				o.erase(o.find("transactions"));
			}
		}

		else
		{
			bytes blockRLP;
			try
			{
				state.sync(bc);
				blockRLP = importByteArray(o["rlp"].get_str());
				bc.import(blockRLP, state.db());
				state.sync(bc);
			}
			// if exception is thrown, RLP is invalid and no blockHeader, Transaction list, or Uncle list should be given
			catch (Exception const& _e)
			{
				cnote << "state sync or block import did throw an exception: " << diagnostic_information(_e);
				BOOST_CHECK(o.count("blockHeader") == 0);
				BOOST_CHECK(o.count("transactions") == 0);
				BOOST_CHECK(o.count("uncleHeaders") == 0);
				continue;
			}
			catch (std::exception const& _e)
			{
				cnote << "state sync or block import did throw an exception: " << _e.what();
				BOOST_CHECK(o.count("blockHeader") == 0);
				BOOST_CHECK(o.count("transactions") == 0);
				BOOST_CHECK(o.count("uncleHeaders") == 0);
				continue;
			}
			catch(...)
			{
				cnote << "state sync or block import did throw an exception\n";
				BOOST_CHECK(o.count("blockHeader") == 0);
				BOOST_CHECK(o.count("transactions") == 0);
				BOOST_CHECK(o.count("uncleHeaders") == 0);
				continue;
			}

			BOOST_REQUIRE(o.count("blockHeader"));

			mObject tObj = o["blockHeader"].get_obj();
			BlockInfo blockHeaderFromFields;
			const bytes c_rlpBytesBlockHeader = createBlockRLPFromFields(tObj);
			const RLP c_blockHeaderRLP(c_rlpBytesBlockHeader);
			blockHeaderFromFields.populateFromHeader(c_blockHeaderRLP, false);

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
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.nonce == blockFromRlp.nonce, "nonce in given RLP not matching the block nonce!");

			BOOST_CHECK_MESSAGE(blockHeaderFromFields == blockFromRlp, "However, blockHeaderFromFields != blockFromRlp!");

			//Check transaction list

			Transactions txsFromField;

			for (auto const& txObj: o["transactions"].get_array())
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

				BOOST_CHECK_MESSAGE(txsFromField[i] == txsFromRlp[i], "transactions in rlp and in transaction field do not match");
			}

			// check uncle list
			BOOST_CHECK_MESSAGE((o["uncleList"].type() == json_spirit::null_type ? 0 : o["uncleList"].get_array().size()) == 0, "Uncle list is not empty, but the genesis block can not have uncles");
		}
	}
}

} }// Namespace Close


BOOST_AUTO_TEST_SUITE(BlockTests)

BOOST_AUTO_TEST_CASE(blValidBlockTest)
{
	dev::test::executeTests("blValidBlockTest", "/BlockTests", dev::test::doBlockTests);
}

BOOST_AUTO_TEST_CASE(blInvalidHeaderTest)
{
	dev::test::executeTests("blInvalidHeaderTest", "/BlockTests", dev::test::doBlockTests);
}

BOOST_AUTO_TEST_CASE(userDefinedFileBl)
{
	dev::test::userDefinedTest("--bltest", dev::test::doBlockTests);
}

BOOST_AUTO_TEST_SUITE_END()
