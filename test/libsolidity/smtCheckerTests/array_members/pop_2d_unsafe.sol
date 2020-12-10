pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f() public {
		a.push();
		a[0].push();
		a[1].pop();
	}
}
// ----
// Warning 2529: (111-121): CHC: Empty array "pop" happens here.\nCounterexample:\na = [[0]]\n\n\n\nTransaction trace:\nconstructor()\nState: a = []\nf()
