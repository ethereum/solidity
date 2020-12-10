pragma experimental SMTChecker;

contract C {
	uint[] a;
	function g() internal {
		a.push();
	}
	function f() public {
		a.pop();
		g();
	}
}
// ----
// Warning 2529: (122-129): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\n\n\nTransaction trace:\nconstructor()\nState: a = []\nf()
