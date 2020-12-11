pragma experimental SMTChecker;

contract C {
	uint256[] x;
	function f(uint256 l) public {
		require(x.length == 0);
		x.push(42);
		x.push(84);
		for(uint256 i = 0; i < l; ++i)
			x.push(23);
		assert(x[0] == 42);
	}
}
// ----
// Warning 4984: (174-177): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
