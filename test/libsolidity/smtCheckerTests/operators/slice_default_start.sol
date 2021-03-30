pragma experimental SMTChecker;

contract C {
	function f(bytes calldata b) external pure {
		require(b.length > 20);
		require(b[0] == 0xff);
		assert(bytes(b[:20]).length == 20);
		assert(bytes(b[:20])[0] == 0xff);
		assert(bytes(b[:20])[5] == 0xff);
	}
}
// ----
// Warning 6328: (183-215): CHC: Assertion violation might happen here.
// Warning 6328: (219-251): CHC: Assertion violation happens here.
// Warning 4661: (183-215): BMC: Assertion violation happens here.
