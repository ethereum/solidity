pragma experimental SMTChecker;
contract C {
	function f(bytes memory data) public pure {
		(uint a1, bytes32 b1, C c1) = abi.decode(data, (uint, bytes32, C));
		(uint a2, bytes32 b2, C c2) = abi.decode(data, (uint, bytes32, C));
		assert(a1 == a2);
		assert(a1 != a2);
	}
}
// ----
// Warning 2072: (102-112): Unused local variable.
// Warning 2072: (114-118): Unused local variable.
// Warning 2072: (172-182): Unused local variable.
// Warning 2072: (184-188): Unused local variable.
// Warning 8364: (155-156): Assertion checker does not yet implement type type(contract C)
// Warning 8364: (225-226): Assertion checker does not yet implement type type(contract C)
// Warning 6328: (252-268): CHC: Assertion violation happens here.\nCounterexample:\n\ndata = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10]\na1 = 7719\nb1 = 5\nc1 = 6\na2 = 7719\nb2 = 5\nc2 = 6\n\nTransaction trace:\nC.constructor()\nC.f([10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10])
// Warning 8364: (155-156): Assertion checker does not yet implement type type(contract C)
// Warning 8364: (225-226): Assertion checker does not yet implement type type(contract C)
