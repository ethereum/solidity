contract C {
	function f(bytes memory data) public pure {
		(uint a1, bytes32 b1, C c1) = abi.decode(data, (uint, bytes32, C));
		(uint a2, bytes32 b2, C c2) = abi.decode(data, (uint, bytes32, C));
		assert(a1 == a2);
		assert(a1 != a2);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (70-80): Unused local variable.
// Warning 2072: (82-86): Unused local variable.
// Warning 2072: (140-150): Unused local variable.
// Warning 2072: (152-156): Unused local variable.
// Warning 6328: (220-236): CHC: Assertion violation happens here.
