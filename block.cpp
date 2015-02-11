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

            BOOST_CHECK_MESSAGE(blockFromFields.stateRoot == theState.rootHash(), "root hash do not match");
			cout << "root hash - no fill in : " << theState.rootHash() << endl;
			cout << "root hash - no fill in  - from block: " << blockFromFields.stateRoot << endl;

            // create new "genesis" block
            RLPStream rlpStream;
            blockFromFields.streamRLP(rlpStream, WithNonce);

            RLPStream block(3);
            block.appendRaw(rlpStream.out());
            block.appendRaw(RLPEmptyList);
            block.appendRaw(RLPEmptyList);

            blockFromFields.verifyInternals(&block.out());

            // construc blockchain
            BlockChain bc(block.out(), string(), true);

            try
            {
                theState.sync(bc);
				bytes blockRLP = importByteArray(o["rlp"].get_str());
				cout << "import block rlp\n";
				bc.import(blockRLP, theState.db());
				cout << "sync with the state\n";
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

			cout << "root hash - no fill in - from state : " << theState.rootHash() << endl;
			cout << " hash - no fill in - from rlp : " << blockFromRlp.hash << endl;
			cout << " hash - no fill in  - from block: " << blockHeaderFromFields.hash << endl;

			//Check the fields restored from RLP to original fields
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.headerHash() == blockFromRlp.headerHash(), "hash in given RLP not matching the block hash!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.parentHash == blockFromRlp.parentHash, "parentHash in given RLP not matching the block parentHash!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.sha3Uncles == blockFromRlp.sha3Uncles, "sha3Uncles in given RLP not matching the block sha3Uncles!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.coinbaseAddress == blockFromRlp.coinbaseAddress,"coinbaseAddress in given RLP not matching the block coinbaseAddress!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.stateRoot == blockFromRlp.stateRoot, "stateRoot in given RLP not matching the block stateRoot!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.transactionsRoot == blockFromRlp.transactionsRoot, "transactionsRoot in given RLP not matching the block transactionsRoot!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.logBloom == blockFromRlp.logBloom, "logBloom in given RLP not matching the block logBloom!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.difficulty == blockFromRlp.difficulty, "difficulty in given RLP not matching the block difficulty!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.number == blockFromRlp.number, "number in given RLP not matching the block number!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.gasLimit == blockFromRlp.gasLimit,"gasLimit in given RLP not matching the block gasLimit!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.gasUsed == blockFromRlp.gasUsed, "gasUsed in given RLP not matching the block gasUsed!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.timestamp == blockFromRlp.timestamp, "timestamp in given RLP not matching the block timestamp!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.extraData == blockFromRlp.extraData, "extraData in given RLP not matching the block extraData!");
			BOOST_CHECK_MESSAGE(blockHeaderFromFields.nonce == blockFromRlp.nonce, "nonce in given RLP not matching the block nonce!");

			BOOST_CHECK_MESSAGE(blockHeaderFromFields == blockFromRlp, "However, blockHeaderFromFields != blockFromRlp!");

        }
        else
        {
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


            // fillin specific --- start

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

                if (!txs.attemptImport(createTransactionFromFields(tx)))
                    cnote << "failed importing transaction\n";
            }

            // update stateRootHash
            blockFromFields.stateRoot = theState.rootHash();
			cout << "root hash1: " << theState.rootHash() << endl;

            // find new valid nonce
            ProofOfWork pow;
            MineInfo ret;
            tie(ret, blockFromFields.nonce) = pow.mine(blockFromFields.headerHash(WithoutNonce), blockFromFields.difficulty, 1000, true, false);
              //---stop

			//update genesis block in json file
			o["genesisBlockHeader"].get_obj()["stateRoot"] = toString(blockFromFields.stateRoot);
			o["genesisBlockHeader"].get_obj()["nonce"] = toString(blockFromFields.nonce);


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



            try
            {
                theState.sync(bc);
				cout << "root hash2: " << theState.rootHash() << endl;
                theState.sync(bc,txs);
				cout << "root hash3: " << theState.rootHash() << endl;
                theState.commitToMine(bc);
				cout << "root hash4: " << theState.rootHash() << endl;
                MineInfo info;
                for (info.completed = false; !info.completed; info = theState.mine()) {}
				cout << "root hash5: " << theState.rootHash() << endl;
                theState.completeMine();
				cout << "root hash6: " << theState.rootHash() << endl;
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
