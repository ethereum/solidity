pragma experimental SMTChecker;
contract C {
	function f(uint x) public pure {
		x = 2;
		while (x > 1) {
			if (x > 10)
				x = 2;
			else
				--x;
		}
		assert(x < 2);
	}
}
