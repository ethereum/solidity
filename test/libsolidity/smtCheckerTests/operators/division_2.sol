pragma experimental SMTChecker;
contract C {
	function f(uint x, uint y) public pure returns (uint) {
		require(y != 0);
		return x / y;
	}
}
