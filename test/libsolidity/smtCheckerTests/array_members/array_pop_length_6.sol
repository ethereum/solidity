pragma experimental SMTChecker;

contract C {
	uint[] a;
	function g() internal view {
		a.length;
	}
	function f() public {
		a.pop();
		g();
	}
}
// ----
// Warning 2529: (127-134): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\n\n\nTransaction trace:\nconstructor()\nState: a = []\nf()
