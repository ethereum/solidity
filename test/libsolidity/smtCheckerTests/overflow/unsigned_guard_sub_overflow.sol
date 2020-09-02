pragma experimental SMTChecker;

contract C  {
	function f(uint x, uint y) public pure returns (uint) {
		require(x >= y);
		return x - y;
	}
}
