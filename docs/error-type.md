# Solidity `error` type

## Motivation

We want to support typed errors in a syntactically equivalent and convenient way like
the programmer can define `event` types.

Example:

```solidity
contract C {
	event HighestBidIncreased(address bidder, uint amount);
	error BiddingFailed(string reason);

	function bid() public payable {
		try attemptBid() {
			emit HighestBidIncreased(msg.sender, msg.value); // Triggering event
		} catch BiddingFailed(reason) {
			// handle error with given reason
		}
    }

	function attemptBid() {
		// assert() should now support a custom secondary parameter to throw custom errors upon
		failed expectations.
		assert(middingPossible, BiddingFailed("Bidding impossible"));
	}
}
```

## Syntax

```bnf
errorDefinition
  : 'error' identifier eventParameterList AnonymousKeyword? ';' ;
```

## Semantic

Errors can be instanciated whereever a variable of a type can be declared.

These errors can be passed around like any other arbitrary value (e.g. function parameters).

What changes is, that those error typed variables can be used as argument in revert() and
as a newly added parameter to assert (that's my proposal).

## catching errors

With the try-catch statement, you can already catch `Error(string)`. This will be extended
to catch errors by other structured types that were declared via the `error` keyword.

The fallback catch will also apply to error types that weren't matched.

Example:

```solidity
contract C {
	event HighestBidIncreased(address bidder, uint amount);

	error BiddingFailed(string reason);
	error SomeOtherError(string text, uint value);

	function bid() public payable {
		try attemptBid() {
			emit HighestBidIncreased(msg.sender, msg.value); // Triggering event
		} catch BiddingFailed(string reason) {
			// handle error with given reason
		} catch SomeOtherError(string reason, uint value) {
			// handle other error with an additional value
		} catch Error(string message) {
			// handle classic error messages
		} catch (bytes data) {
			// parametrized default
		}
	}
}
```

## The current `Error(string message)` type.

Currently, exceptions can be caught via `catch Error(string text)`. This behaviour will be retained
by providing the default error type `Error`.

## Code Generation

Whatever ABI encoder is selected, this one will be used to encode/decode the error in
assert/revert/catch.







