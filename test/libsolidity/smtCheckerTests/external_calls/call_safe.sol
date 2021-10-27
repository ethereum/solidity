contract C {
	uint x;
	function f(address a) public {
		(bool s, bytes memory data) = a.call("");
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (57-63): Unused local variable.
// Warning 2072: (65-82): Unused local variable.
// Info 1180: Contract invariant(s) for :C:\n(x <= 0)\nReentrancy property(ies) for :C:\n((!(x <= 0) || (x' <= 0)) && ((<errorCode> <= 0) || !(x <= 0)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(x == 0)\n
