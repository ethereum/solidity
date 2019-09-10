pragma experimental SMTChecker;

contract C
{
	modifier m(uint x) {
		require(x == 2);
		_;
		return;
	}

	modifier n(uint x) {
		require(x == 3);
		_;
	}

	function f(uint x) m(x) n(x) public pure {
		assert(x == 3);
	}
}
// ----
