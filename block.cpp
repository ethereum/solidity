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
    BOOST_REQUIRE(_tObj.count("parentHash") > 0);
    BOOST_REQUIRE(_tObj.count("uncleHash") > 0);
    BOOST_REQUIRE(_tObj.count("coinbase") > 0);
    BOOST_REQUIRE(_tObj.count("stateRoot") > 0);
    BOOST_REQUIRE(_tObj.count("transactionsTrie")> 0);
    BOOST_REQUIRE(_tObj.count("receiptTrie") > 0);
    BOOST_REQUIRE(_tObj.count("bloom") > 0);
    BOOST_REQUIRE(_tObj.count("difficulty") > 0);
    BOOST_REQUIRE(_tObj.count("number") > 0);
    BOOST_REQUIRE(_tObj.count("gasLimit")> 0);
    BOOST_REQUIRE(_tObj.count("gasUsed") > 0);
    BOOST_REQUIRE(_tObj.count("timestamp") > 0);
    BOOST_REQUIRE(_tObj.count("extraData") > 0);
    BOOST_REQUIRE(_tObj.count("nonce") > 0);

    // construct RLP of the given block
    cout << "done with require\n";
    RLPStream rlpStream;
    rlpStream.appendList(14);
    cout << "increate aha1\n";
    rlpStream <<  h256(_tObj["parentHash"].get_str()) << h256(_tObj["uncleHash"].get_str()) << Address(_tObj["coinbase"].get_str());
    rlpStream << h256(_tObj["stateRoot"].get_str()) << h256(_tObj["transactionsTrie"].get_str()) << h256(_tObj["receiptTrie"].get_str());
    rlpStream << LogBloom(_tObj["bloom"].get_str()) << u256(_tObj["difficulty"].get_str()) << u256(_tObj["number"].get_str());
    rlpStream << u256(_tObj["gasLimit"].get_str()) << u256(_tObj["gasUsed"].get_str()) << u256(_tObj["timestamp"].get_str());
    rlpStream << importByteArray(_tObj["extraData"].get_str()) << h256(_tObj["nonce"].get_str());
    cout << "done createBlockRLPFromFields" << endl;

    return rlpStream.out();
}

void doBlockTests(json_spirit::mValue& _v, bool _fillin)
{
    for (auto& i: _v.get_obj())
    {
        cerr << i.first << endl;
        mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("genesisBlockHeader") > 0);

		// construct RLP of the genesis block
		bytes blockRLP = createBlockRLPFromFields(o["genesisBlockHeader"].get_obj());
		RLP myRLP(blockRLP);
		BlockInfo blockFromFields;

		try
		{
			blockFromFields.populateFromHeader(myRLP, false);
		}
		catch (Exception const& _e)
		{
			cnote << "block construction did throw an exception: " << diagnostic_information(_e);
			BOOST_ERROR("Failed block construction Test with Exception: " << _e.what());
			return;
		}
		catch (std::exception const& _e)
		{
			cnote << "block construction did throw an exception: " << _e.what();
			BOOST_ERROR("Failed block construction Test with Exception: " << _e.what());
			return;
		}
		catch(...)
		{
			cnote << "block construction did throw an unknown exception\n";
			return;
		}

		BOOST_REQUIRE(o.count("pre") > 0);

		ImportTest importer(o["pre"].get_obj());
		State theState(Address(), OverlayDB(), BaseState::Empty);
		importer.importState(o["pre"].get_obj(), theState);

		// commit changes to DB
		theState.commit();

		if (_fillin)
			blockFromFields.stateRoot = theState.rootHash();
		else
			BOOST_CHECK_MESSAGE(blockFromFields.stateRoot == theState.rootHash(), "root hash do not match");

		if (_fillin)
		{
			// find new valid nonce
			ProofOfWork pow;
			MineInfo ret;
			tie(ret, blockFromFields.nonce) = pow.mine(blockFromFields.headerHash(WithoutNonce), blockFromFields.difficulty, 1000, true, false);

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
			BOOST_REQUIRE(o.count("transactions") > 0);

			TransactionQueue txs;

			for (auto const& txObj: o["transactions"].get_array())
			{
				mObject tx = txObj.get_obj();
				BOOST_REQUIRE(tx.count("nonce") > 0);
				BOOST_REQUIRE(tx.count("gasPrice") > 0);
				BOOST_REQUIRE(tx.count("gasLimit") > 0);
				BOOST_REQUIRE(tx.count("to") > 0);
				BOOST_REQUIRE(tx.count("value") > 0);
				BOOST_REQUIRE(tx.count("v") > 0);
				BOOST_REQUIRE(tx.count("r") > 0);
				BOOST_REQUIRE(tx.count("s") > 0);
				BOOST_REQUIRE(tx.count("data") > 0);

				//Transaction txFromFields(createRLPStreamFromTransactionFields(tx).out(), CheckSignature::Sender);

				if (!txs.attemptImport(&createRLPStreamFromTransactionFields(tx).out()))
					cnote << "failed importing transaction\n";
			}

			try
			{
				theState.sync(bc);
				theState.sync(bc,txs);
				theState.commitToMine(bc);
				MineInfo info;
				for (info.completed = false; !info.completed; info = theState.mine()) {}
				theState.completeMine();
			}
			catch (Exception const& _e)
			{
				cnote << "state sync or mining did throw an exception: " << diagnostic_information(_e);
			}
			catch (std::exception const& _e)
			{
				cnote << "state sync or mining did throw an exception: " << _e.what();
			}

			o["rlp"] = "0x" + toHex(theState.blockData());

			// write block header

			mObject oBlockHeader;
			BlockInfo current_BlockHeader = theState.info();
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
		}

		else
		{
			try
			{
				theState.sync(bc);
				bytes blockRLP = importByteArray(o["rlp"].get_str());
				bc.import(blockRLP, theState.db());
				theState.sync(bc);
			}
			// if exception is thrown, RLP is invalid and not blockHeader, Transaction list, and Uncle list should be given
			catch (Exception const& _e)
			{
				cnote << "state sync or block import did throw an exception: " << diagnostic_information(_e);
				BOOST_CHECK(o.count("blockHeader") == 0);
				BOOST_CHECK(o.count("transactions") == 0);
				BOOST_CHECK(o.count("uncleHeaders") == 0);
			}
			catch (std::exception const& _e)
			{
				cnote << "state sync or block import did throw an exception: " << _e.what();
				BOOST_CHECK(o.count("blockHeader") == 0);
				BOOST_CHECK(o.count("transactions") == 0);
				BOOST_CHECK(o.count("uncleHeaders") == 0);
			}
			catch(...)
			{
				cnote << "state sync or block import did throw an exception\n";
				BOOST_CHECK(o.count("blockHeader") == 0);
				BOOST_CHECK(o.count("transactions") == 0);
				BOOST_CHECK(o.count("uncleHeaders") == 0);
			}


			// if yes, check parameters in blockHeader
			// check transaction list
			// check uncle list

			BOOST_REQUIRE(o.count("blockHeader") > 0);

			mObject tObj = o["blockHeader"].get_obj();
			BlockInfo blockHeaderFromFields;
			const bytes rlpBytesBlockHeader = createBlockRLPFromFields(tObj);
			RLP blockHeaderRLP(rlpBytesBlockHeader);
			blockHeaderFromFields.populateFromHeader(blockHeaderRLP, false);

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
				BOOST_REQUIRE(tx.count("nonce") > 0);
				BOOST_REQUIRE(tx.count("gasPrice") > 0);
				BOOST_REQUIRE(tx.count("gasLimit") > 0);
				BOOST_REQUIRE(tx.count("to") > 0);
				BOOST_REQUIRE(tx.count("value") > 0);
				BOOST_REQUIRE(tx.count("v") > 0);
				BOOST_REQUIRE(tx.count("r") > 0);
				BOOST_REQUIRE(tx.count("s") > 0);
				BOOST_REQUIRE(tx.count("data") > 0);

				Transaction t(createRLPStreamFromTransactionFields(tx).out(), CheckSignature::Sender);
				txsFromField.push_back(t);
			}

			Transactions txsFromRlp;
			bytes blockRLP2 = importByteArray(o["rlp"].get_str());
			RLP root(blockRLP2);
			for (auto const& tr: root[1])
			{
				Transaction tx(tr.data(), CheckSignature::Sender);
				txsFromRlp.push_back(tx);
			}

			cout << "size of pending transactions: " << txsFromRlp.size() << endl;

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

				BOOST_CHECK_MESSAGE(txsFromField[i] == txsFromRlp[i], "however, transactions in rlp and in field do not match");
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

BOOST_AUTO_TEST_CASE(userDefinedFileBl)
{
    dev::test::userDefinedTest("--bltest", dev::test::doBlockTests);
}

BOOST_AUTO_TEST_SUITE_END()
