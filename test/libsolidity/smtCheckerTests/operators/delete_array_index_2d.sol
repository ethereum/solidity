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
		a[2][1] = 4;
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
// Warning 6368: (264-268): CHC: Out of bounds access might happen here.
// Warning 6368: (264-271): CHC: Out of bounds access might happen here.
// Warning 6368: (316-320): CHC: Out of bounds access might happen here.
// Warning 6368: (341-345): CHC: Out of bounds access might happen here.
// Warning 6368: (341-348): CHC: Out of bounds access might happen here.
// Warning 6328: (334-354): CHC: Assertion violation might happen here.
// Warning 6368: (365-369): CHC: Out of bounds access might happen here.
// Warning 6368: (365-372): CHC: Out of bounds access might happen here.
// Warning 6328: (358-378): CHC: Assertion violation might happen here.
// Warning 4661: (358-378): BMC: Assertion violation happens here.
