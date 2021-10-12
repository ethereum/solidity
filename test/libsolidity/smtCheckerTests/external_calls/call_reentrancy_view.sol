contract C {
	uint x;
	function s(uint _x) public view {
		x == _x;
	}
	function f(address a) public {
		(bool s, bytes memory data) = a.call("");
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 2519: (106-112): This declaration shadows an existing declaration.
// Warning 2072: (106-112): Unused local variable.
// Warning 2072: (114-131): Unused local variable.
// Info 1180: Contract invariant(s) for :C:\n(x <= 0)\nReentrancy property(ies) for :C:\n((!(x <= 0) || (x' <= 0)) && ((<errorCode> <= 0) || !(x <= 0)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(x == 0)\n
