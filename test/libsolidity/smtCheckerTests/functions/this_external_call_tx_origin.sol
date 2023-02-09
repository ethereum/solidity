contract C {

	function test() view public {
		require(address(this) != tx.origin);
		assert(!this.g());
	}

	function g() view public returns (bool) {
		return msg.sender == tx.origin;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
