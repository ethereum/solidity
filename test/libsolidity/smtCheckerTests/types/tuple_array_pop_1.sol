contract C {
	int[] a;
	function f() public { (a).pop();}
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (46-55): CHC: Empty array "pop" happens here.\nCounterexample:\na = []\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
