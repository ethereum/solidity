pragma experimental SMTChecker;
contract C {
	int[] a;
	function f() public { (((((a))))).pop();}
}
// ----
// Warning 2529: (78-95): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
