pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f(uint[] memory x, uint y) public {
		a.push(x);
		a[0].push(y);
		assert(a[0][a[0].length - 1] == y);
	}
}
