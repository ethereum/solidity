/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Ben Edgington <ben@benjaminion.xyz>
 * @date 2017
 * Tests for the deployed ENS Registry implementation written in LLL
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <test/liblll/ExecutionFramework.h>
#include <liblll/Compiler.h>

#define ACCOUNT(n)    h256(account(n), h256::AlignRight)

using namespace std;
using namespace dev::lll;
using namespace dev::test;

namespace dev
{
namespace lll
{
namespace test
{

namespace
{

static char const* ensCode = R"DELIMITER(
;;; ---------------------------------------------------------------------------
;;; @title The Ethereum Name Service registry.
;;; @author Daniel Ellison <daniel@syrinx.net>

(seq

  ;; --------------------------------------------------------------------------
  ;; Constant definitions.

  ;; Memory layout.
  (def 'node-bytes  0x00)
  (def 'label-bytes 0x20)
  (def 'call-result 0x40)

  ;; Struct: Record
  (def 'resolver 0x00) ; address
  (def 'owner    0x20) ; address
  (def 'ttl      0x40) ; uint64

  ;; Precomputed function IDs.
  (def 'get-node-owner    0x02571be3) ; owner(bytes32)
  (def 'get-node-resolver 0x0178b8bf) ; resolver(bytes32)
  (def 'get-node-ttl      0x16a25cbd) ; ttl(bytes32)
  (def 'set-node-owner    0x5b0fc9c3) ; setOwner(bytes32,address)
  (def 'set-subnode-owner 0x06ab5923) ; setSubnodeOwner(bytes32,bytes32,address)
  (def 'set-node-resolver 0x1896f70a) ; setResolver(bytes32,address)
  (def 'set-node-ttl      0x14ab9038) ; setTTL(bytes32,uint64)

  ;; Jumping here causes an EVM error.
  (def 'invalid-location 0x02)

  ;; --------------------------------------------------------------------------
  ;; @notice Shifts the leftmost 4 bytes of a 32-byte number right by 28 bytes.
  ;; @param input A 32-byte number.

  (def 'shift-right (input)
    (div input (exp 2 224)))

  ;; --------------------------------------------------------------------------
  ;; @notice Determines whether the supplied function ID matches a known
  ;;         function hash and executes <code-body> if so.
  ;; @dev The function ID is in the leftmost four bytes of the call data.
  ;; @param function-hash The four-byte hash of a known function signature.
  ;; @param code-body The code to run in the case of a match.

  (def 'function (function-hash code-body)
    (when (= (shift-right (calldataload 0x00)) function-hash)
      code-body))

  ;; --------------------------------------------------------------------------
  ;; @notice Calculates record location for the node and label passed in.
  ;; @param node The parent node.
  ;; @param label The hash of the subnode label.

  (def 'get-record (node label)
    (seq
      (mstore node-bytes node)
      (mstore label-bytes label)
      (sha3 node-bytes 64)))

  ;; --------------------------------------------------------------------------
  ;; @notice Retrieves owner from node record.
  ;; @param node Get owner of this node.

  (def 'get-owner (node)
    (sload (+ node owner)))

  ;; --------------------------------------------------------------------------
  ;; @notice Stores new owner in node record.
  ;; @param node Set owner of this node.
  ;; @param new-owner New owner of this node.

  (def 'set-owner (node new-owner)
    (sstore (+ node owner) new-owner))

  ;; --------------------------------------------------------------------------
  ;; @notice Stores new subnode owner in node record.
  ;; @param node Set owner of this node.
  ;; @param label The hash of the label specifying the subnode.
  ;; @param new-owner New owner of the subnode.

  (def 'set-subowner (node label new-owner)
    (sstore (+ (get-record node label) owner) new-owner))

  ;; --------------------------------------------------------------------------
  ;; @notice Retrieves resolver from node record.
  ;; @param node Get resolver of this node.

  (def 'get-resolver (node)
    (sload node))

  ;; --------------------------------------------------------------------------
  ;; @notice Stores new resolver in node record.
  ;; @param node Set resolver of this node.
  ;; @param new-resolver New resolver for this node.

  (def 'set-resolver (node new-resolver)
    (sstore node new-resolver))

  ;; --------------------------------------------------------------------------
  ;; @notice Retrieves TTL From node record.
  ;; @param node Get TTL of this node.

  (def 'get-ttl (node)
    (sload (+ node ttl)))

  ;; --------------------------------------------------------------------------
  ;; @notice Stores new TTL in node record.
  ;; @param node Set TTL of this node.
  ;; @param new-resolver New TTL for this node.

  (def 'set-ttl (node new-ttl)
    (sstore (+ node ttl) new-ttl))

  ;; --------------------------------------------------------------------------
  ;; @notice Checks that the caller is the node owner.
  ;; @param node Check owner of this node.

  (def 'only-node-owner (node)
    (when (!= (caller) (get-owner node))
      (jump invalid-location)))

  ;; --------------------------------------------------------------------------
  ;; INIT

  ;; Set the owner of the root node (0x00) to the deploying account.
  (set-owner 0x00 (caller))

  ;; --------------------------------------------------------------------------
  ;; CODE

  (returnlll
    (seq

      ;; ----------------------------------------------------------------------
      ;; @notice Returns the address of the resolver for the specified node.
      ;; @dev Signature: resolver(bytes32)
      ;; @param node Return this node's resolver.
      ;; @return The associated resolver.

      (def 'node (calldataload 0x04))

      (function get-node-resolver
        (seq

          ;; Get the node's resolver and save it.
          (mstore call-result (get-resolver node))

          ;; Return result.
          (return call-result 32)))

      ;; ----------------------------------------------------------------------
      ;; @notice Returns the address that owns the specified node.
      ;; @dev Signature: owner(bytes32)
      ;; @param node Return this node's owner.
      ;; @return The associated address.

      (def 'node (calldataload 0x04))

      (function get-node-owner
        (seq

          ;; Get the node's owner and save it.
          (mstore call-result (get-owner node))

          ;; Return result.
          (return call-result 32)))

      ;; ----------------------------------------------------------------------
      ;; @notice Returns the TTL of a node and any records associated with it.
      ;; @dev Signature: ttl(bytes32)
      ;; @param node Return this node's TTL.
      ;; @return The node's TTL.

      (def 'node (calldataload 0x04))

      (function get-node-ttl
        (seq

          ;; Get the node's TTL and save it.
          (mstore call-result (get-ttl node))

          ;; Return result.
          (return call-result 32)))

      ;; ----------------------------------------------------------------------
      ;; @notice Transfers ownership of a node to a new address. May only be
      ;;         called by the current owner of the node.
      ;; @dev Signature: setOwner(bytes32,address)
      ;; @param node The node to transfer ownership of.
      ;; @param new-owner The address of the new owner.

      (def 'node (calldataload 0x04))
      (def 'new-owner (calldataload 0x24))

      (function set-node-owner
        (seq (only-node-owner node)

          ;; Transfer ownership by storing passed-in address.
          (set-owner node new-owner)

          ;; Emit an event about the transfer.
          ;; Transfer(bytes32 indexed node, address owner);
          (mstore call-result new-owner)
          (log2 call-result 32
              (sha3 0x00 (lit 0x00 "Transfer(bytes32,address)")) node)

          ;; Nothing to return.
          (stop)))

      ;; ----------------------------------------------------------------------
      ;; @notice Transfers ownership of a subnode to a new address. May only be
      ;;         called by the owner of the parent node.
      ;; @dev Signature: setSubnodeOwner(bytes32,bytes32,address)
      ;; @param node The parent node.
      ;; @param label The hash of the label specifying the subnode.
      ;; @param new-owner The address of the new owner.

      (def 'node (calldataload 0x04))
      (def 'label (calldataload 0x24))
      (def 'new-owner (calldataload 0x44))

      (function set-subnode-owner
        (seq (only-node-owner node)

          ;; Transfer ownership by storing passed-in address.
          (set-subowner node label new-owner)

          ;; Emit an event about the transfer.
          ;; NewOwner(bytes32 indexed node, bytes32 indexed label, address owner);
          (mstore call-result new-owner)
          (log3 call-result 32
              (sha3 0x00 (lit 0x00 "NewOwner(bytes32,bytes32,address)"))
              node label)

          ;; Nothing to return.
          (stop)))

      ;; ----------------------------------------------------------------------
      ;; @notice Sets the resolver address for the specified node.
      ;; @dev Signature: setResolver(bytes32,address)
      ;; @param node The node to update.
      ;; @param new-resolver The address of the resolver.

      (def 'node (calldataload 0x04))
      (def 'new-resolver (calldataload 0x24))

      (function set-node-resolver
        (seq (only-node-owner node)

          ;; Transfer ownership by storing passed-in address.
          (set-resolver node new-resolver)

          ;; Emit an event about the change of resolver.
          ;; NewResolver(bytes32 indexed node, address resolver);
          (mstore call-result new-resolver)
          (log2 call-result 32
              (sha3 0x00 (lit 0x00 "NewResolver(bytes32,address)")) node)

          ;; Nothing to return.
          (stop)))

      ;; ----------------------------------------------------------------------
      ;; @notice Sets the TTL for the specified node.
      ;; @dev Signature: setTTL(bytes32,uint64)
      ;; @param node The node to update.
      ;; @param ttl The TTL in seconds.

      (def 'node (calldataload 0x04))
      (def 'new-ttl (calldataload 0x24))

      (function set-node-ttl
        (seq (only-node-owner node)

          ;; Set new TTL by storing passed-in time.
          (set-ttl node new-ttl)

          ;; Emit an event about the change of TTL.
          ;; NewTTL(bytes32 indexed node, uint64 ttl);
          (mstore call-result new-ttl)
          (log2 call-result 32
              (sha3 0x00 (lit 0x00 "NewTTL(bytes32,uint64)")) node)

          ;; Nothing to return.
          (stop)))

      ;; ----------------------------------------------------------------------
      ;; @notice Fallback: No functions matched the function ID provided.

      (jump invalid-location)))

)
)DELIMITER";

static unique_ptr<bytes> s_compiledEns;

class LLLENSTestFramework: public LLLExecutionFramework
{
protected:
	void deployEns()
	{
		if (!s_compiledEns)
		{
			vector<string> errors;
			s_compiledEns.reset(new bytes(compileLLL(ensCode, dev::test::Options::get().evmVersion(), dev::test::Options::get().optimize, &errors)));
			BOOST_REQUIRE(errors.empty());
		}
		sendMessage(*s_compiledEns, true);
		BOOST_REQUIRE(m_transactionSuccessful);
		BOOST_REQUIRE(!m_output.empty());
	}

};

}

// Test suite for the deployed ENS Registry implementation written in LLL
BOOST_FIXTURE_TEST_SUITE(LLLENS, LLLENSTestFramework)

BOOST_AUTO_TEST_CASE(creation)
{
	deployEns();

	// Root node 0x00 should initially be owned by the deploying account, account(0).
	BOOST_CHECK(callContractFunction("owner(bytes32)", 0x00) == encodeArgs(ACCOUNT(0)));
}

BOOST_AUTO_TEST_CASE(transfer_ownership)
{
	deployEns();

	// Transfer ownership of root node from account(0) to account(1).
	BOOST_REQUIRE(callContractFunction("setOwner(bytes32,address)", 0x00, ACCOUNT(1)) == encodeArgs());

	// Check that an event was raised and contents are correct.
	BOOST_REQUIRE(numLogs() == 1);
	BOOST_CHECK(logData(0) == encodeArgs(ACCOUNT(1)));
	BOOST_REQUIRE(numLogTopics(0) == 2);
	BOOST_CHECK(logTopic(0, 0) == keccak256(string("Transfer(bytes32,address)")));
	BOOST_CHECK(logTopic(0, 1) == u256(0x00));

	// Verify that owner of 0x00 is now account(1).
	BOOST_CHECK(callContractFunction("owner(bytes32)", 0x00) == encodeArgs(ACCOUNT(1)));
}

BOOST_AUTO_TEST_CASE(transfer_ownership_fail)
{
	deployEns();

	// Try to steal ownership of node 0x01
	BOOST_REQUIRE(callContractFunction("setOwner(bytes32,address)", 0x01, ACCOUNT(0)) == encodeArgs());

	// Verify that owner of 0x01 remains the default zero address
	BOOST_CHECK(callContractFunction("owner(bytes32)", 0x01) == encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(set_resolver)
{
	deployEns();

	// Set resolver of root node to account(1).
	BOOST_REQUIRE(callContractFunction("setResolver(bytes32,address)", 0x00, ACCOUNT(1)) == encodeArgs());

	// Check that an event was raised and contents are correct.
	BOOST_REQUIRE(numLogs() == 1);
	BOOST_CHECK(logData(0) == encodeArgs(ACCOUNT(1)));
	BOOST_REQUIRE(numLogTopics(0) == 2);
	BOOST_CHECK(logTopic(0, 0) == keccak256(string("NewResolver(bytes32,address)")));
	BOOST_CHECK(logTopic(0, 1) == u256(0x00));

	// Verify that the resolver is changed to account(1).
	BOOST_CHECK(callContractFunction("resolver(bytes32)", 0x00) == encodeArgs(ACCOUNT(1)));
}

BOOST_AUTO_TEST_CASE(set_resolver_fail)
{
	deployEns();

	// Try to set resolver of node 0x01, which is not owned by account(0).
	BOOST_REQUIRE(callContractFunction("setResolver(bytes32,address)", 0x01, ACCOUNT(0)) == encodeArgs());

	// Verify that the resolver of 0x01 remains default zero address.
	BOOST_CHECK(callContractFunction("resolver(bytes32)", 0x01) == encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(set_ttl)
{
	deployEns();

	// Set ttl of root node to 3600.
	BOOST_REQUIRE(callContractFunction("setTTL(bytes32,uint64)", 0x00, 3600) == encodeArgs());

	// Check that an event was raised and contents are correct.
	BOOST_REQUIRE(numLogs() == 1);
	BOOST_CHECK(logData(0) == encodeArgs(3600));
	BOOST_REQUIRE(numLogTopics(0) == 2);
	BOOST_CHECK(logTopic(0, 0) == keccak256(string("NewTTL(bytes32,uint64)")));
	BOOST_CHECK(logTopic(0, 1) == u256(0x00));

	// Verify that the TTL has been set.
	BOOST_CHECK(callContractFunction("ttl(bytes32)", 0x00) == encodeArgs(3600));
}

BOOST_AUTO_TEST_CASE(set_ttl_fail)
{
	deployEns();

	// Try to set TTL of node 0x01, which is not owned by account(0).
	BOOST_REQUIRE(callContractFunction("setTTL(bytes32,uint64)", 0x01, 3600) == encodeArgs());

	// Verify that the TTL of node 0x01 has not changed from the default.
	BOOST_CHECK(callContractFunction("ttl(bytes32)", 0x01) == encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(create_subnode)
{
	deployEns();

	// Set ownership of "eth" sub-node to account(1)
	BOOST_REQUIRE(callContractFunction("setSubnodeOwner(bytes32,bytes32,address)", 0x00, keccak256(string("eth")), ACCOUNT(1)) == encodeArgs());

	// Check that an event was raised and contents are correct.
	BOOST_REQUIRE(numLogs() == 1);
	BOOST_CHECK(logData(0) == encodeArgs(ACCOUNT(1)));
	BOOST_REQUIRE(numLogTopics(0) == 3);
	BOOST_CHECK(logTopic(0, 0) == keccak256(string("NewOwner(bytes32,bytes32,address)")));
	BOOST_CHECK(logTopic(0, 1) == u256(0x00));
	BOOST_CHECK(logTopic(0, 2) == keccak256(string("eth")));

	// Verify that the sub-node owner is now account(1).
	u256 namehash = keccak256(h256(0x00).asBytes() + keccak256("eth").asBytes());
	BOOST_CHECK(callContractFunction("owner(bytes32)", namehash) == encodeArgs(ACCOUNT(1)));
}

BOOST_AUTO_TEST_CASE(create_subnode_fail)
{
	deployEns();

	// Send account(1) some ether for gas.
	sendEther(account(1), 1000 * ether);
	BOOST_REQUIRE(balanceAt(account(1)) >= 1000 * ether);

	// account(1) tries to set ownership of the "eth" sub-node.
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunction("setSubnodeOwner(bytes32,bytes32,address)", 0x00, keccak256(string("eth")), ACCOUNT(1)) == encodeArgs());

	// Verify that the sub-node owner remains at default zero address.
	u256 namehash = keccak256(h256(0x00).asBytes() + keccak256("eth").asBytes());
	BOOST_CHECK(callContractFunction("owner(bytes32)", namehash) == encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(fallback)
{
	deployEns();

	// Call fallback - should just abort via jump to invalid location.
	BOOST_CHECK(callFallback() == encodeArgs());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
