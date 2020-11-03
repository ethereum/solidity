pragma experimental SMTChecker;

contract C {
	function f() public pure {
		uint x = type(uint256).max - 1;
		assert(x == 2**256 - 2);
		assert(~1 == -2);
	}
}
// ----
