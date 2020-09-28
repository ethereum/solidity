pragma experimental SMTChecker;

contract C {
	function f(uint i) public pure {
		bytes4 x = 0x01020304;
		require(i > 3);
		assert(x[i] == 0x00);
		i = 5;
		assert(x[i] == 0);
		assert(x[i] != 0);
	}
}
// ----
// Warning 6328: (125-145): CHC: Assertion violation happens here.
// Warning 6328: (158-175): CHC: Assertion violation happens here.
// Warning 6328: (179-196): CHC: Assertion violation happens here.
