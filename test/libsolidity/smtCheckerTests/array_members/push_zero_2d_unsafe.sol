pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[0].push();
		assert(a[a.length - 1][0] == 100);
	}
}
// ----
// Warning 6328: (111-144): CHC: Assertion violation happens here.\nCounterexample:\na = [[0]]\n\n\n\nTransaction trace:\nconstructor()\nState: a = []\nf()
