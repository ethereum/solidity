pragma experimental SMTChecker;

contract C {
	uint[] a;
	function f(uint x) public {
		require(a.length < 100000);
		a.push(x);
		assert(a[a.length - 1] == x);
	}
}
