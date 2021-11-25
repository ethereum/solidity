contract C {
	function f(address _a) public {
		uint x = address(this).balance;
		_a.call("");
		assert(address(this).balance == x); // should fail
		assert(address(this).balance >= x); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 9302: (82-93): Return value of low-level calls not used.
// Warning 6328: (97-131): CHC: Assertion violation happens here.
// Info 1180: Reentrancy property(ies) for :C:\n(!(<errorCode> >= 2) && (((:var 1).balances[address(this)] + ((- 1) * (:var 0).balances[address(this)])) <= 0))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(address(this).balance == x)\n<errorCode> = 2 -> Assertion failed at assert(address(this).balance >= x)\n
