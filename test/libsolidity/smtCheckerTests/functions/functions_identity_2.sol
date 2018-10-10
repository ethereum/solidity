pragma experimental SMTChecker;
contract C
{
	function h(uint x) public pure returns (uint) {
		return k(x);
	}

	function k(uint x) public pure returns (uint) {
		return x;
	}
	function g() public pure {
		uint x;
		x = h(2);
		assert(x > 0);
	}
}

