pragma experimental SMTChecker;

contract C
{
	uint[][] a;
	constructor() {
		init();
	}
	function init() internal {
		a.push();
		a.push();
		a.push();
		a[1].push();
		a[1].push();
		a[2].push();
		a[2].push();
	}
	function f(bool b) public {
		a[1][1] = 512;
		if (b)
			delete a;
		else
			delete a[2];
		init();
		assert(a[2][3] == 0);
		assert(a[1][1] == 0);
	}
}
// ====
// SMTSolvers: z3
// ----
// Warning 6368: (247-251): CHC: Out of bounds access might happen here.
// Warning 6368: (247-254): CHC: Out of bounds access might happen here.
// Warning 6368: (301-305): CHC: Out of bounds access might happen here.
// Warning 6368: (326-330): CHC: Out of bounds access might happen here.
// Warning 6368: (326-333): CHC: Out of bounds access might happen here.
// Warning 6328: (319-339): CHC: Assertion violation might happen here.
// Warning 6368: (350-354): CHC: Out of bounds access might happen here.
// Warning 6368: (350-357): CHC: Out of bounds access might happen here.
// Warning 6328: (343-363): CHC: Assertion violation might happen here.
// Warning 4661: (343-363): BMC: Assertion violation happens here.
