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
/** @file transaction.cpp
 * @author Dmitrii Khokhlov <winsvega@mail.ru>
 * @date 2015
 * Transaaction test functions.
 */

#include "TestHelper.h"
using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;

namespace dev {  namespace test {

RLPStream createRLPStreamFromTransactionFields(mObject& _tObj)
{
	//Construct Rlp of the given transaction
	RLPStream rlpStream;
	rlpStream.appendList(_tObj.size());

	if (_tObj.count("nonce") > 0)
		rlpStream << bigint(_tObj["nonce"].get_str());

	if (_tObj.count("gasPrice") > 0)
		rlpStream << bigint(_tObj["gasPrice"].get_str());

	if (_tObj.count("gasLimit") > 0)
		rlpStream << bigint(_tObj["gasLimit"].get_str());

	if (_tObj.count("to") > 0)
	{
		if (_tObj["to"].get_str().empty())
			rlpStream << "";
		else
			rlpStream << importByteArray(_tObj["to"].get_str());
	}

	if (_tObj.count("value") > 0)
		rlpStream << bigint(_tObj["value"].get_str());

	if (_tObj.count("data") > 0)
		rlpStream << importData(_tObj);

	if (_tObj.count("v") > 0)
		rlpStream << bigint(_tObj["v"].get_str());

	if (_tObj.count("r") > 0)
		rlpStream << bigint(_tObj["r"].get_str());

	if (_tObj.count("s") > 0)
		rlpStream << bigint(_tObj["s"].get_str());

	if (_tObj.count("extrafield") > 0)
		rlpStream << bigint(_tObj["extrafield"].get_str());

	return rlpStream;
}

void doTransactionTests(json_spirit::mValue& _v, bool _fillin)
{
	for (auto& i: _v.get_obj())
	{
		cerr << i.first << endl;
		mObject& o = i.second.get_obj();

		if (_fillin == false)
		{
			BOOST_REQUIRE(o.count("rlp") > 0);
			Transaction txFromRlp;
			try
			{
				bytes stream = importByteArray(o["rlp"].get_str());
				RLP rlp(stream);
				txFromRlp = Transaction(rlp.data(), CheckSignature::Sender);
				if (!txFromRlp.signature().isValid())
					BOOST_THROW_EXCEPTION(Exception() << errinfo_comment("transaction from RLP signature is invalid") );
			}
			catch(...)
			{
				BOOST_CHECK_MESSAGE(o.count("transaction") == 0, "A transaction object should not be defined because the RLP is invalid!");
				return;
			}

			BOOST_REQUIRE(o.count("transaction") > 0);

			mObject tObj = o["transaction"].get_obj();
			Transaction txFromFields(createRLPStreamFromTransactionFields(tObj).out(), CheckSignature::Sender);

			//Check the fields restored from RLP to original fields
			BOOST_CHECK_MESSAGE(txFromFields.data() == txFromRlp.data(), "Data in given RLP not matching the Transaction data!");
			BOOST_CHECK_MESSAGE(txFromFields.value() == txFromRlp.value(), "Value in given RLP not matching the Transaction value!");
			BOOST_CHECK_MESSAGE(txFromFields.gasPrice() == txFromRlp.gasPrice(), "GasPrice in given RLP not matching the Transaction gasPrice!");
			BOOST_CHECK_MESSAGE(txFromFields.gas() == txFromRlp.gas(),"Gas in given RLP not matching the Transaction gas!");
			BOOST_CHECK_MESSAGE(txFromFields.nonce() == txFromRlp.nonce(),"Nonce in given RLP not matching the Transaction nonce!");
			BOOST_CHECK_MESSAGE(txFromFields.receiveAddress() == txFromRlp.receiveAddress(), "Receive address in given RLP not matching the Transaction 'to' address!");
			BOOST_CHECK_MESSAGE(txFromFields.sender() == txFromRlp.sender(), "Transaction sender address in given RLP not matching the Transaction 'vrs' signature!");
			BOOST_CHECK_MESSAGE(txFromFields == txFromRlp, "However, txFromFields != txFromRlp!");
			BOOST_REQUIRE (o.count("sender") > 0);

			Address addressReaded = Address(o["sender"].get_str());
			BOOST_CHECK_MESSAGE(txFromFields.sender() == addressReaded || txFromRlp.sender() == addressReaded, "Signature address of sender does not match given sender address!");

		}
		else
		{
			BOOST_REQUIRE(o.count("transaction") > 0);
			mObject tObj = o["transaction"].get_obj();

			//Construct Rlp of the given transaction
			RLPStream rlpStream = createRLPStreamFromTransactionFields(tObj);
			o["rlp"] = "0x" + toHex(rlpStream.out());

			try
			{
				Transaction txFromFields(rlpStream.out(), CheckSignature::Sender);
				if (!txFromFields.signature().isValid())
					BOOST_THROW_EXCEPTION(Exception() << errinfo_comment("transaction from RLP signature is invalid") );

				o["sender"] = toString(txFromFields.sender());
			}
			catch(...)
			{
				o.erase(o.find("transaction"));
			}
		}
	}//for
}//doTransactionTests
} }// Namespace Close


BOOST_AUTO_TEST_SUITE(TransactionTests)

BOOST_AUTO_TEST_CASE(TransactionTest)
{
	dev::test::executeTests("ttTransactionTest", "/TransactionTests", dev::test::doTransactionTests);
}

BOOST_AUTO_TEST_CASE(tt10mbDataField)
{
	dev::test::executeTests("tt10mbDataField", "/TransactionTests", dev::test::doTransactionTests);
}

BOOST_AUTO_TEST_CASE(ttCreateTest)
{
	for (int i = 1; i < boost::unit_test::framework::master_test_suite().argc; ++i)
	{
		string arg = boost::unit_test::framework::master_test_suite().argv[i];
		if (arg == "--createtest")
		{
			if (boost::unit_test::framework::master_test_suite().argc <= i + 2)
			{
				cnote << "usage: ./testeth --createtest <PathToConstructor> <PathToDestiny>\n";
				return;
			}
			try
			{
				cnote << "Populating tests...";
				json_spirit::mValue v;
				string s = asString(dev::contents(boost::unit_test::framework::master_test_suite().argv[i + 1]));
				BOOST_REQUIRE_MESSAGE(s.length() > 0, "Content of " + (string)boost::unit_test::framework::master_test_suite().argv[i + 1] + " is empty.");
				json_spirit::read_string(s, v);
				dev::test::doTransactionTests(v, true);
				writeFile(boost::unit_test::framework::master_test_suite().argv[i + 2], asBytes(json_spirit::write_string(v, true)));
			}
			catch (Exception const& _e)
			{
				BOOST_ERROR("Failed transaction test with Exception: " << diagnostic_information(_e));
			}
			catch (std::exception const& _e)
			{
				BOOST_ERROR("Failed transaction test with Exception: " << _e.what());
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(userDefinedFileTT)
{
	dev::test::userDefinedTest("--ttTest", dev::test::doTransactionTests);
}

BOOST_AUTO_TEST_SUITE_END()
