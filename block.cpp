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
    rlpStream << h256(_tObj["stateRoot"].get_str()) << h256(_tObj["transactionsTrie"].get_str()) << Address(_tObj["receiptTrie"].get_str());
    rlpStream << LogBloom(_tObj["bloom"].get_str()) << u256(_tObj["difficulty"].get_str()) << u256(_tObj["number"].get_str());
    rlpStream << u256(_tObj["gasLimit"].get_str()) << u256(_tObj["gasUsed"].get_str()) << u256(_tObj["timestamp"].get_str());
    rlpStream << importByteArray(_tObj["extraData"].get_str()) << h256(_tObj["nonce"].get_str());

    return rlpStream.out();
}

void doBlockTests(json_spirit::mValue& _v, bool _fillin)
{
    for (auto& i: _v.get_obj())
    {
        cerr << i.first << endl;
        mObject& o = i.second.get_obj();

        if (_fillin == false)
        {
            // TODO

            //			BOOST_REQUIRE(o.count("rlp") > 0);
            //			const bytes rlpReaded = importByteArray(o["rlp"].get_str());
            //			RLP myRLP(rlpReaded);
            //			BlockInfo blockFromRlp;

            //			try
            //			{
            //				blockFromRlp.populateFromHeader(myRLP, false);
            //				//blockFromRlp.verifyInternals(rlpReaded);
            //			}
            //			catch(Exception const& _e)
            //			{
            //				cnote << "block construction did throw an exception: " << diagnostic_information(_e);
            //				BOOST_ERROR("Failed block construction Test with Exception: " << _e.what());
            //				BOOST_CHECK_MESSAGE(o.count("block") == 0, "A block object should not be defined because the block RLP is invalid!");
            //				return;
            //			}

            //			BOOST_REQUIRE(o.count("block") > 0);

            //			mObject tObj = o["block"].get_obj();
            //			BlockInfo blockFromFields;
            //			const bytes rlpreade2 = createBlockRLPFromFields(tObj);
            //			RLP mysecondRLP(rlpreade2);
            //			blockFromFields.populateFromHeader(mysecondRLP, false);

            //			//Check the fields restored from RLP to original fields
            //			BOOST_CHECK_MESSAGE(blockFromFields.hash == blockFromRlp.hash, "hash in given RLP not matching the block hash!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.parentHash == blockFromRlp.parentHash, "parentHash in given RLP not matching the block parentHash!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.sha3Uncles == blockFromRlp.sha3Uncles, "sha3Uncles in given RLP not matching the block sha3Uncles!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.coinbaseAddress == blockFromRlp.coinbaseAddress,"coinbaseAddress in given RLP not matching the block coinbaseAddress!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.stateRoot == blockFromRlp.stateRoot, "stateRoot in given RLP not matching the block stateRoot!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.transactionsRoot == blockFromRlp.transactionsRoot, "transactionsRoot in given RLP not matching the block transactionsRoot!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.logBloom == blockFromRlp.logBloom, "logBloom in given RLP not matching the block logBloom!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.difficulty == blockFromRlp.difficulty, "difficulty in given RLP not matching the block difficulty!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.number == blockFromRlp.number, "number in given RLP not matching the block number!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.gasLimit == blockFromRlp.gasLimit,"gasLimit in given RLP not matching the block gasLimit!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.gasUsed == blockFromRlp.gasUsed, "gasUsed in given RLP not matching the block gasUsed!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.timestamp == blockFromRlp.timestamp, "timestamp in given RLP not matching the block timestamp!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.extraData == blockFromRlp.extraData, "extraData in given RLP not matching the block extraData!");
            //			BOOST_CHECK_MESSAGE(blockFromFields.nonce == blockFromRlp.nonce, "nonce in given RLP not matching the block nonce!");

            //			BOOST_CHECK_MESSAGE(blockFromFields == blockFromRlp, "However, blockFromFields != blockFromRlp!");

        }
        else
        {
            BOOST_REQUIRE(o.count("block") > 0);

            // construct Rlp of the given block
            bytes blockRLP = createBlockRLPFromFields(o["block"].get_obj());
            RLP myRLP(blockRLP);
            BlockInfo blockFromFields;
            cout << "blockFromFields diff:" << blockFromFields.difficulty << endl;

            try
            {

                blockFromFields.populateFromHeader(myRLP, false);
                //blockFromFields.verifyInternals(blockRLP);
            }
            catch (Exception const& _e)
            {
                cnote << "block construction did throw an exception: " << diagnostic_information(_e);
                BOOST_ERROR("Failed block construction Test with Exception: " << _e.what());
                o.erase(o.find("block"));
            }
            catch (std::exception const& _e)
            {
                cnote << "block construction did throw an exception: " << _e.what();
                BOOST_ERROR("Failed block construction Test with Exception: " << _e.what());
                o.erase(o.find("block"));
            }
            catch(...)
            {
                cnote << "block construction did throw an unknown exception\n";
                o.erase(o.find("block"));
            }

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
                cout << "attempt to import transaction\n";
                txs.attemptImport(createTransactionFromFields(tx));
            }
            cout << "done importing txs\n";

            BOOST_REQUIRE(o.count("pre") > 0);
            // create state

            ImportTest importer(o["pre"].get_obj());
            State theState(Address(), OverlayDB(), BaseState::Empty);
            importer.importState(o["pre"].get_obj(), theState);

            cout << "current state diff 1 : " << theState.info().difficulty << endl;
            cout << "balance of a94f5374fce5edbc8e2a8697c15331677e6ebf0b: " << theState.balance(Address("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b")) << endl;
            theState.commit();
            cout << "balance of a94f5374fce5edbc8e2a8697c15331677e6ebf0b: " << theState.balance(Address("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b")) << endl;
            blockFromFields.stateRoot = theState.rootHash();
            cout << "stateRoot: " << blockFromFields.stateRoot << endl;

            // find new nonce
            ProofOfWork pow;
            MineInfo ret;
            tie(ret, blockFromFields.nonce) = pow.mine(blockFromFields.headerHash(WithoutNonce), blockFromFields.difficulty, 1000, true, false);

            // create new "genesis" block"

            RLPStream rlpStream;
            blockFromFields.streamRLP(rlpStream, WithNonce);

            RLPStream block(3);
            block.appendRaw(rlpStream.out());
            block.appendRaw(RLPEmptyList);
            block.appendRaw(RLPEmptyList);
            //return block.out();
            cout << "my genesis hash: " << sha3(RLP(block.out())[0].data()) << endl;

            cout << "construct bc\n";
            BlockChain bc(block.out(), std::string(), true);
            cout << "pre difficulty: " << blockFromFields.difficulty << endl;

            try
            {
                //cout << "sync state with pre block" << endl;
                theState.sync(bc);
                cout << "sync bc and txs in state\n";
                cout << "balance of a94f5374fce5edbc8e2a8697c15331677e6ebf0b: " << theState.balance(Address("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b")) << endl;
                cout << "current state diff: " << theState.info().difficulty << endl;
                theState.sync(bc,txs);
                cout << "current state diff: " << theState.info().difficulty << endl;
                // lock
                cout << "commit to mine\n";
                theState.commitToMine(bc);  // will call uncommitToMine if a repeat.
                cout << "current state diff: " << theState.info().difficulty << endl;
                // unlock
                MineInfo info;
                cout << "mine...\n";
                for (info.completed = false; !info.completed; info = theState.mine()) {}
                cout << "done mining, completeMine\n";
                // lock
                theState.completeMine();
                cout << "current state diff: " << theState.info().difficulty << endl;
                // unlock

                cout << "new block: " << theState.blockData() << endl << theState.info() << endl;
                cout << "current diff: " << theState.info().difficulty << endl;
            }
            catch (Exception const& _e)
            {
                cnote << "state sync did throw an exception: " << diagnostic_information(_e);
            }
            catch (std::exception const& _e)
            {
                cnote << "state sync did throw an exception: " << _e.what();
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

            mArray aUncleList; // as of now, our parent is always the genesis block, so we can not have uncles. That will change.
            o["uncleHeaders"] = aUncleList;
        }
    }
}

} }// Namespace Close


BOOST_AUTO_TEST_SUITE(BlockTests)

//BOOST_AUTO_TEST_CASE(blFirstTest)
//{
//    dev::test::executeTests("blFirstTest", "/BlockTests", dev::test::doBlockTests);
//}

BOOST_AUTO_TEST_CASE(blValidBlockTest)
{
    dev::test::executeTests("blValidBlockTest", "/BlockTests", dev::test::doBlockTests);
}

//BOOST_AUTO_TEST_CASE(ttCreateTest)
//{
//	for (int i = 1; i < boost::unit_test::framework::master_test_suite().argc; ++i)
//	{
//		string arg = boost::unit_test::framework::master_test_suite().argv[i];
//		if (arg == "--createtest")
//		{
//			if (boost::unit_test::framework::master_test_suite().argc <= i + 2)
//			{
//				cnote << "usage: ./testeth --createtest <PathToConstructor> <PathToDestiny>\n";
//				return;
//			}
//			try
//			{
//				cnote << "Populating tests...";
//				json_spirit::mValue v;
//				string s = asString(dev::contents(boost::unit_test::framework::master_test_suite().argv[i + 1]));
//				BOOST_REQUIRE_MESSAGE(s.length() > 0, "Content of " + (string)boost::unit_test::framework::master_test_suite().argv[i + 1] + " is empty.");
//				json_spirit::read_string(s, v);
//				dev::test::doBlockTests(v, true);
//				writeFile(boost::unit_test::framework::master_test_suite().argv[i + 2], asBytes(json_spirit::write_string(v, true)));
//			}
//			catch (Exception const& _e)
//			{
//				BOOST_ERROR("Failed block test with Exception: " << diagnostic_information(_e));
//			}
//			catch (std::exception const& _e)
//			{
//				BOOST_ERROR("Failed block test with Exception: " << _e.what());
//			}
//		}
//	}
//}

//BOOST_AUTO_TEST_CASE(userDefinedFileTT)
//{
//	dev::test::userDefinedTest("--bltest", dev::test::doBlockTests);
//}

BOOST_AUTO_TEST_SUITE_END()
