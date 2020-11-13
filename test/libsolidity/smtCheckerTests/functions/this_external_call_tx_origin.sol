pragma experimental SMTChecker;

contract C {

	function test() view public {
		require(address(this) != tx.origin);
		assert(!this.g());
	}

	function g() view public returns (bool) {
		return msg.sender == tx.origin;
	}
}
// ----
