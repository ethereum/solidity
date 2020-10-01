pragma experimental SMTChecker;

contract C {
	function f(bytes calldata b) external pure {
		require(b[10] == 0xff);
		assert(bytes(b[10:20]).length == 10);
		assert(bytes(b[10:20])[0] == 0xff);
		assert(bytes(b[10:20])[5] == 0xff);
	}
}
// ----
// Warning 6328: (198-232): CHC: Assertion violation happens here.
