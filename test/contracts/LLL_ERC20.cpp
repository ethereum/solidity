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
 * Tests for an ERC20 token implementation written in LLL
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <test/liblll/ExecutionFramework.h>
#include <liblll/Compiler.h>

#define TOKENSUPPLY   100000
#define TOKENDECIMALS 2
#define TOKENSYMBOL   "BEN"
#define TOKENNAME     "Ben Token"
#define ACCOUNT(n)    h256(account(n), h256::AlignRight)
#define SUCCESS       encodeArgs(1)

using namespace std;
using namespace dev::eth;

namespace dev
{
namespace lll
{
namespace test
{

namespace
{

static char const* erc20Code = R"DELIMITER(
(seq

  ;; --------------------------------------------------------------------------
  ;; CONSTANTS

  ;; Token parameters.
  ;;   0x40 is a "magic number" - the text of the string is placed here
  ;;   when returning the string to the caller. See return-string below.
  (def 'token-name-string   (lit 0x40 "Ben Token"))
  (def 'token-symbol-string (lit 0x40 "BEN"))
  (def 'token-decimals 2)
  (def 'token-supply 100000) ; 1000.00 total tokens

  ;; Booleans
  (def 'false 0)
  (def 'true  1)

  ;; Memory layout.
  (def 'mem-ret    0x00) ; Fixed due to compiler macro for return.
  (def 'mem-func   0x00) ; No conflict with mem-ret, so re-use.
  (def 'mem-keccak 0x00) ; No conflict with mem-func or mem-ret, so re-use.
  (def 'scratch0   0x20)
  (def 'scratch1   0x40)

  ;; Precomputed function IDs.
  (def 'get-name         0x06fdde03) ; name()
  (def 'get-symbol       0x95d89b41) ; symbol()
  (def 'get-decimals     0x313ce567) ; decimals()
  (def 'get-total-supply 0x18160ddd) ; totalSupply()
  (def 'get-balance-of   0x70a08231) ; balanceOf(address)
  (def 'transfer         0xa9059cbb) ; transfer(address,uint256)
  (def 'transfer-from    0x23b872dd) ; transferFrom(address,address,uint256)
  (def 'approve          0x095ea7b3) ; approve(address,uint256)
  (def 'get-allowance    0xdd62ed3e) ; allowance(address,address)

  ;; Event IDs
  (def 'transfer-event-id ; Transfer(address,address,uint256)
    0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef)

  (def 'approval-event-id ; Approval(address,address,uint256)
    0x8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925)

  ;; --------------------------------------------------------------------------
  ;; UTILITIES

  ;; --------------------------------------------------------------------------
  ;;  The following define the key data-structures:
  ;;    - balance(addr) => value
  ;;    - allowance(addr,addr) => value

  ;; Balances are stored at s[owner_addr].
  (def 'balance (address) address)

  ;; Allowances are stored at s[owner_addr + keccak256(spender_addr)]
  ;;   We use a crypto function here to avoid any situation where
  ;;   approve(me, spender) can be abused to do approve(target, me).
  (def 'allowance (owner spender)
    (seq
      (mstore mem-keccak spender)
      (keccak256 mem-keccak 0x20)))

  ;; --------------------------------------------------------------------------
  ;; For convenience we have macros to refer to function arguments

  (def 'arg1 (calldataload 0x04))
  (def 'arg2 (calldataload 0x24))
  (def 'arg3 (calldataload 0x44))

  ;; --------------------------------------------------------------------------
  ;; Revert is a soft return that does not consume the remaining gas.
  ;;   We use it when rejecting invalid user input.
  ;;
  ;; Note: The REVERT opcode will be implemented in Metropolis (EIP 140).
  ;;   Meanwhile it just causes an invalid instruction exception (similar
  ;;   to a "throw" in Solidity).  When fully implemented, Revert could be
  ;;   use to return error codes, or even messages.

  (def 'revert () (revert 0 0))

  ;; --------------------------------------------------------------------------
  ;; Macro for returning string names.
  ;;   Compliant with the ABI format for strings.

  (def 'return-string (string-literal)
    (seq
      (mstore 0x00 0x20)           ; Points to our string's memory location
      (mstore 0x20 string-literal) ; Length. String itself is copied to 0x40.
      (return 0x00 (& (+ (mload 0x20) 0x5f) (~ 0x1f)))))
                                   ; Round return up to 32 byte boundary

  ;; --------------------------------------------------------------------------
  ;; Convenience macro for raising Events

  (def 'event3 (id addr1 addr2 value)
    (seq
      (mstore scratch0 value)
      (log3 scratch0 0x20 id addr1 addr2)))

  ;; --------------------------------------------------------------------------
  ;; Determines whether the stored function ID matches a known
  ;;   function hash and executes <code-body> if so.
  ;; @param function-hash The four-byte hash of a known function signature.
  ;; @param code-body The code to run in the case of a match.

  (def 'function (function-hash code-body)
    (when (= (mload mem-func) function-hash)
      code-body))

  ;; --------------------------------------------------------------------------
  ;; Gets the function ID and stores it in memory for reference.
  ;;   The function ID is in the leftmost four bytes of the call data.

  (def 'uses-functions
    (mstore
      mem-func
      (shr (calldataload 0x00) 224)))

  ;; --------------------------------------------------------------------------
  ;; GUARDS

  ;; --------------------------------------------------------------------------
  ;; Checks that ensure that each function is called with the right
  ;;   number of arguments. For one thing this addresses the "ERC20
  ;;   short address attack". For another, it stops me making
  ;;   mistakes while testing. We use these only on the non-constant functions.

  (def 'has-one-arg    (unless (= 0x24 (calldatasize)) (revert)))
  (def 'has-two-args   (unless (= 0x44 (calldatasize)) (revert)))
  (def 'has-three-args (unless (= 0x64 (calldatasize)) (revert)))

  ;; --------------------------------------------------------------------------
  ;; Check that addresses have only 160 bits and revert if not.
  ;;   We use these input type-checks on the non-constant functions.

  (def 'is-address (addr)
    (when
      (shr addr 160)
      (revert)))

  ;; --------------------------------------------------------------------------
  ;; Check that transfer values are smaller than total supply and
  ;;   revert if not. This should effectively exclude negative values.

  (def 'is-value (value)
    (when (> value token-supply) (revert)))

  ;; --------------------------------------------------------------------------
  ;; Will revert if sent any Ether. We use the macro immediately so as
  ;;   to abort if sent any Ether during contract deployment.

  (def 'not-payable
    (when (callvalue) (revert)))

  not-payable

  ;; --------------------------------------------------------------------------
  ;; INITIALISATION
  ;;
  ;; Assign all tokens initially to the owner of the contract.

  (sstore (balance (caller)) token-supply)

  ;; --------------------------------------------------------------------------
  ;; CONTRACT CODE

  (returnlll
    (seq not-payable uses-functions

      ;; ----------------------------------------------------------------------
      ;; Getter for the name of the token.
      ;; @abi name() constant returns (string)
      ;; @return The token name as a string.

      (function get-name
        (return-string token-name-string))

      ;; ----------------------------------------------------------------------
      ;; Getter for the symbol of the token.
      ;; @abi symbol() constant returns (string)
      ;; @return The token symbol as a string.

      (function get-symbol
        (return-string token-symbol-string))

      ;; ----------------------------------------------------------------------
      ;; Getter for the number of decimals assigned to the token.
      ;; @abi decimals() constant returns (uint256)
      ;; @return The token decimals.

      (function get-decimals
        (return token-decimals))

      ;; ----------------------------------------------------------------------
      ;; Getter for the total token supply.
      ;; @abi totalSupply() constant returns (uint256)
      ;; @return The token supply.

      (function get-total-supply
        (return token-supply))

      ;; ----------------------------------------------------------------------
      ;; Returns the account balance of another account.
      ;; @abi balanceOf(address) constant returns (uint256)
      ;; @param owner The address of the account's owner.
      ;; @return The account balance.

      (function get-balance-of
        (seq

          (def 'owner arg1)

          (return (sload (balance owner)))))

      ;; ----------------------------------------------------------------------
      ;; Transfers _value amount of tokens to address _to. The command
      ;;   should throw if the _from account balance has not enough
      ;;   tokens to spend.
      ;; @abi transfer(address, uint256) returns (bool)
      ;; @param to The account to receive the tokens.
      ;; @param value The quantity of tokens to transfer.
      ;; @return Success (true). Other outcomes result in a Revert.

      (function transfer
        (seq has-two-args (is-address arg1) (is-value arg2)

          (def 'to    arg1)
          (def 'value arg2)

          (when value ; value == 0 is a no-op
            (seq

              ;; The caller's balance. Save in memory for efficiency.
              (mstore scratch0 (sload (balance (caller))))

              ;; Revert if the caller's balance is not sufficient.
              (when (> value (mload scratch0))
                (revert))

              ;; Make the transfer
              ;; It would be good to check invariants (sum of balances).
              (sstore (balance (caller)) (- (mload scratch0) value))
              (sstore (balance to) (+ (sload (balance to)) value))

              ;; Event - Transfer(address,address,uint256)
              (event3 transfer-event-id (caller) to value)))

          (return true)))

      ;; ----------------------------------------------------------------------
      ;; Send _value amount of tokens from address _from to address _to
      ;; @abi transferFrom(address,address,uint256) returns (bool)
      ;; @param from The account to send the tokens from.
      ;; @param to The account to receive the tokens.
      ;; @param value The quantity of tokens to transfer.
      ;; @return Success (true). Other outcomes result in a Revert.

      (function transfer-from
        (seq has-three-args (is-address arg1) (is-address arg2) (is-value arg3)

          (def 'from  arg1)
          (def 'to    arg2)
          (def 'value arg3)

          (when value ; value == 0 is a no-op

            (seq

              ;; Save data to memory for efficiency.
              (mstore scratch0 (sload (balance from)))
              (mstore scratch1 (sload (allowance from (caller))))

              ;; Revert if not enough funds, or not enough approved.
              (when
                (||
                  (> value (mload scratch0))
                  (> value (mload scratch1)))
                (revert))

              ;; Make the transfer and update allowance.
              (sstore (balance from) (- (mload scratch0) value))
              (sstore (balance to) (+ (sload (balance to)) value))
              (sstore (allowance from (caller)) (- (mload scratch1) value))

              ;; Event - Transfer(address,address,uint256)
              (event3 transfer-event-id from to value)))

          (return true)))

      ;; ----------------------------------------------------------------------
      ;; Allows _spender to withdraw from your account multiple times,
      ;;   up to the _value amount. If this function is called again it
      ;;   overwrites the current allowance with _value.
      ;; @abi approve(address,uint256) returns (bool)
      ;; @param spender The withdrawing account having its limit set.
      ;; @param value The maximum allowed amount.
      ;; @return Success (true). Other outcomes result in a Revert.

      (function approve
        (seq has-two-args (is-address arg1) (is-value arg2)

          (def 'spender arg1)
          (def 'value   arg2)

          ;; Force users set the allowance to 0 before setting it to
          ;; another value for the same spender. Prevents this attack:
          ;; https://docs.google.com/document/d/1YLPtQxZu1UAvO9cZ1O2RPXBbT0mooh4DYKjA_jp-RLM
          (when
            (&& value (sload (allowance (caller) spender)))
            (revert))

          (sstore (allowance (caller) spender) value)

          ;; Event - Approval(address,address,uint256)
          (event3 approval-event-id (caller) spender value)

          (return true)))

      ;; ----------------------------------------------------------------------
      ;; Returns the amount which _spender is still allowed to withdraw
      ;;   from _owner.
      ;; @abi allowance(address,address) constant returns (uint256)
      ;; @param owner The owning account.
      ;; @param spender The withdrawing account.
      ;; @return The allowed amount remaining.

      (function get-allowance
        (seq

          (def 'owner   arg1)
          (def 'spender arg2)

          (return (sload (allowance owner spender)))))

      ;; ----------------------------------------------------------------------
      ;; Fallback: No functions matched the function ID provided.

      (revert)))
  )
)DELIMITER";

static unique_ptr<bytes> s_compiledErc20;

class LLLERC20TestFramework: public LLLExecutionFramework
{
protected:
	void deployErc20()
	{
		if (!s_compiledErc20)
		{
			vector<string> errors;
			s_compiledErc20.reset(new bytes(compileLLL(erc20Code, dev::test::Options::get().evmVersion(), dev::test::Options::get().optimize, &errors)));
			BOOST_REQUIRE(errors.empty());
		}
		sendMessage(*s_compiledErc20, true);
		BOOST_REQUIRE(m_transactionSuccessful);
		BOOST_REQUIRE(!m_output.empty());
	}

};

}

// Test suite for an ERC20 contract written in LLL.
BOOST_FIXTURE_TEST_SUITE(LLLERC20, LLLERC20TestFramework)

BOOST_AUTO_TEST_CASE(creation)
{
	deployErc20();

	// All tokens are initially assigned to the contract creator.
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY));
}

BOOST_AUTO_TEST_CASE(constants)
{
	deployErc20();

	BOOST_CHECK(callContractFunction("totalSupply()") == encodeArgs(TOKENSUPPLY));
	BOOST_CHECK(callContractFunction("decimals()") == encodeArgs(TOKENDECIMALS));
	BOOST_CHECK(callContractFunction("symbol()") == encodeDyn(string(TOKENSYMBOL)));
	BOOST_CHECK(callContractFunction("name()") == encodeDyn(string(TOKENNAME)));
}

BOOST_AUTO_TEST_CASE(send_value)
{
	deployErc20();

	// Send value to the contract. Should always fail.
	m_sender = account(0);
	auto contractBalance = balanceAt(m_contractAddress);

	// Fallback: check value is not transferred.
	BOOST_CHECK(callFallbackWithValue(42) != SUCCESS);
	BOOST_CHECK(balanceAt(m_contractAddress) == contractBalance);

	// Transfer: check nothing happened.
	BOOST_CHECK(callContractFunctionWithValue("transfer(address,uint256)", ACCOUNT(1), 100, 42) != SUCCESS);
	BOOST_CHECK(balanceAt(m_contractAddress) == contractBalance);
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(1)) == encodeArgs(0));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY));
}

BOOST_AUTO_TEST_CASE(transfer)
{
	deployErc20();

	// Transfer 100 tokens from account(0) to account(1).
	int transfer = 100;
	m_sender = account(0);
	BOOST_CHECK(callContractFunction("transfer(address,uint256)", ACCOUNT(1), u256(transfer)) == SUCCESS);
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY - transfer));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(1)) == encodeArgs(transfer));
}

BOOST_AUTO_TEST_CASE(transfer_from)
{
	deployErc20();

	// Approve account(1) to transfer up to 1000 tokens from account(0).
	int allow = 1000;
	m_sender = account(0);
	BOOST_REQUIRE(callContractFunction("approve(address,uint256)", ACCOUNT(1), u256(allow)) == SUCCESS);
	BOOST_REQUIRE(callContractFunction("allowance(address,address)", ACCOUNT(0), ACCOUNT(1)) == encodeArgs(allow));

	// Send account(1) some ether for gas.
	sendEther(account(1), 1000 * ether);
	BOOST_REQUIRE(balanceAt(account(1)) >= 1000 * ether);

	// Transfer 300 tokens from account(0) to account(2); check that the allowance decreases.
	int transfer = 300;
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunction("transferFrom(address,address,uint256)", ACCOUNT(0), ACCOUNT(2), u256(transfer)) == SUCCESS);
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(2)) == encodeArgs(transfer));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY - transfer));
	BOOST_CHECK(callContractFunction("allowance(address,address)", ACCOUNT(0), ACCOUNT(1)) == encodeArgs(allow - transfer));
}

BOOST_AUTO_TEST_CASE(transfer_event)
{
	deployErc20();

	// Transfer 1000 tokens from account(0) to account(1).
	int transfer = 1000;
	m_sender = account(0);
	BOOST_REQUIRE(callContractFunction("transfer(address,uint256)", ACCOUNT(1), u256(transfer)) == SUCCESS);

	// Check that a Transfer event was recorded and contents are correct.
	BOOST_REQUIRE(m_logs.size() == 1);
	BOOST_CHECK(m_logs[0].data == encodeArgs(transfer));
	BOOST_REQUIRE(m_logs[0].topics.size() == 3);
	BOOST_CHECK(m_logs[0].topics[0] == keccak256(string("Transfer(address,address,uint256)")));
	BOOST_CHECK(m_logs[0].topics[1] == ACCOUNT(0));
	BOOST_CHECK(m_logs[0].topics[2] == ACCOUNT(1));
}

BOOST_AUTO_TEST_CASE(transfer_zero_no_event)
{
	deployErc20();

	// Transfer 0 tokens from account(0) to account(1). This is a no-op.
	int transfer = 0;
	m_sender = account(0);
	BOOST_REQUIRE(callContractFunction("transfer(address,uint256)", ACCOUNT(1), u256(transfer)) == SUCCESS);

	// Check that no Event was recorded.
	BOOST_CHECK(m_logs.size() == 0);

	// Check that balances have not changed.
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY - transfer));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(1)) == encodeArgs(transfer));
}

BOOST_AUTO_TEST_CASE(approval_and_transfer_events)
{
	deployErc20();

	// Approve account(1) to transfer up to 10000 tokens from account(0).
	int allow = 10000;
	m_sender = account(0);
	BOOST_REQUIRE(callContractFunction("approve(address,uint256)", ACCOUNT(1), u256(allow)) == SUCCESS);

	// Check that an Approval event was recorded and contents are correct.
	BOOST_REQUIRE(m_logs.size() == 1);
	BOOST_CHECK(m_logs[0].data == encodeArgs(allow));
	BOOST_REQUIRE(m_logs[0].topics.size() == 3);
	BOOST_CHECK(m_logs[0].topics[0] == keccak256(string("Approval(address,address,uint256)")));
	BOOST_CHECK(m_logs[0].topics[1] == ACCOUNT(0));
	BOOST_CHECK(m_logs[0].topics[2] == ACCOUNT(1));

	// Send account(1) some ether for gas.
	sendEther(account(1), 1000 * ether);
	BOOST_REQUIRE(balanceAt(account(1)) >= 1000 * ether);

	// Transfer 3000 tokens from account(0) to account(2); check that the allowance decreases.
	int transfer = 3000;
	m_sender = account(1);
	BOOST_REQUIRE(callContractFunction("transferFrom(address,address,uint256)", ACCOUNT(0), ACCOUNT(2), u256(transfer)) == SUCCESS);

	// Check that a Transfer event was recorded and contents are correct.
	BOOST_REQUIRE(m_logs.size() == 1);
	BOOST_CHECK(m_logs[0].data == encodeArgs(transfer));
	BOOST_REQUIRE(m_logs[0].topics.size() == 3);
	BOOST_CHECK(m_logs[0].topics[0] == keccak256(string("Transfer(address,address,uint256)")));
	BOOST_CHECK(m_logs[0].topics[1] == ACCOUNT(0));
	BOOST_CHECK(m_logs[0].topics[2] == ACCOUNT(2));
}

BOOST_AUTO_TEST_CASE(invalid_transfer_1)
{
	deployErc20();

	// Transfer more than the total supply; ensure nothing changes.
	int transfer = TOKENSUPPLY + 1;
	m_sender = account(0);
	BOOST_CHECK(callContractFunction("transfer(address,uint256)", ACCOUNT(1), u256(transfer)) != SUCCESS);
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(1)) == encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(invalid_transfer_2)
{
	deployErc20();

	// Separate transfers that together exceed initial balance.
	int transfer = 1 + TOKENSUPPLY / 2;
	m_sender = account(0);

	// First transfer should succeed.
	BOOST_REQUIRE(callContractFunction("transfer(address,uint256)", ACCOUNT(1), u256(transfer)) == SUCCESS);
	BOOST_REQUIRE(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY - transfer));
	BOOST_REQUIRE(callContractFunction("balanceOf(address)", ACCOUNT(1)) == encodeArgs(transfer));

	// Second transfer should fail.
	BOOST_CHECK(callContractFunction("transfer(address,uint256)", ACCOUNT(1), u256(transfer)) != SUCCESS);
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY - transfer));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(1)) == encodeArgs(transfer));
}

BOOST_AUTO_TEST_CASE(invalid_transfer_from)
{
	deployErc20();

	// TransferFrom without approval.
	int transfer = 300;

	// Send account(1) some ether for gas.
	m_sender = account(0);
	sendEther(account(1), 1000 * ether);
	BOOST_REQUIRE(balanceAt(account(1)) >= 1000 * ether);

	// Try the transfer; ensure nothing changes.
	m_sender = account(1);
	BOOST_CHECK(callContractFunction("transferFrom(address,address,uint256)", ACCOUNT(0), ACCOUNT(2), u256(transfer)) != SUCCESS);
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(2)) == encodeArgs(0));
	BOOST_CHECK(callContractFunction("balanceOf(address)", ACCOUNT(0)) == encodeArgs(TOKENSUPPLY));
	BOOST_CHECK(callContractFunction("allowance(address,address)", ACCOUNT(0), ACCOUNT(1)) == encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(invalid_reapprove)
{
	deployErc20();

	m_sender = account(0);

	// Approve account(1) to transfer up to 1000 tokens from account(0).
	int allow1 = 1000;
	BOOST_REQUIRE(callContractFunction("approve(address,uint256)", ACCOUNT(1), u256(allow1)) == SUCCESS);
	BOOST_REQUIRE(callContractFunction("allowance(address,address)", ACCOUNT(0), ACCOUNT(1)) == encodeArgs(allow1));

	// Now approve account(1) to transfer up to 500 tokens from account(0).
	// Should fail (we need to reset allowance to 0 first).
	int allow2 = 500;
	BOOST_CHECK(callContractFunction("approve(address,uint256)", ACCOUNT(1), u256(allow2)) != SUCCESS);
	BOOST_CHECK(callContractFunction("allowance(address,address)", ACCOUNT(0), ACCOUNT(1)) == encodeArgs(allow1));
}

BOOST_AUTO_TEST_CASE(bad_data)
{
	deployErc20();

	m_sender = account(0);

	// Correct data: transfer(address _to, 1).
	sendMessage((bytes)fromHex("a9059cbb") + (bytes)fromHex("000000000000000000000000123456789a123456789a123456789a123456789a") + encodeArgs(1), false, 0);
	BOOST_CHECK(m_transactionSuccessful);
	BOOST_CHECK(m_output == SUCCESS);

	// Too little data (address is truncated by one byte).
	sendMessage((bytes)fromHex("a9059cbb") + (bytes)fromHex("000000000000000000000000123456789a123456789a123456789a12345678") + encodeArgs(1), false, 0);
	BOOST_CHECK(!m_transactionSuccessful);
	BOOST_CHECK(m_output != SUCCESS);

	// Too much data (address is extended with a zero byte).
	sendMessage((bytes)fromHex("a9059cbb") + (bytes)fromHex("000000000000000000000000123456789a123456789a123456789a123456789a00") + encodeArgs(1), false, 0);
	BOOST_CHECK(!m_transactionSuccessful);
	BOOST_CHECK(m_output != SUCCESS);

	// Invalid address (a bit above the 160th is set).
	sendMessage((bytes)fromHex("a9059cbb") + (bytes)fromHex("000000000000000000000100123456789a123456789a123456789a123456789a") + encodeArgs(1), false, 0);
	BOOST_CHECK(!m_transactionSuccessful);
	BOOST_CHECK(m_output != SUCCESS);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
