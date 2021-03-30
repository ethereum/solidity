pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function f() public {
		a.push();
		a.push();
		a[0].push();
		a[1].pop();
	}
}
// ----
// Warning 2529: (123-133): CHC: Empty array "pop" happens here.\nCounterexample:\na = [[0], []]\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.f()
