pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[0].push();
		assert(a[a.length - 1][0] == 0);
	}
}
