interface I {
	function ext() external;
}

contract C {
	function f(I _i) public {
		uint x = address(this).balance;
		_i.ext();
		assert(address(this).balance == x); // should fail
		assert(address(this).balance >= x); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 1218: (131-165): CHC: Error trying to invoke SMT solver.
// Warning 6328: (131-165): CHC: Assertion violation might happen here.
// Info 1180: Reentrancy property(ies) for :C:\n(!(<errorCode> >= 2) && (((:var 0).balances[address(this)] + ((- 1) * (:var 1).balances[address(this)])) >= 0))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(address(this).balance == x)\n<errorCode> = 2 -> Assertion failed at assert(address(this).balance >= x)\n
// Warning 4661: (131-165): BMC: Assertion violation happens here.
