contract C
{
	uint[][] a;
	constructor() {
		init();
	}
	function init() internal {
		a.push();
		a.push();
		a[0].push();
		a[1].push();
	}
	function f(bool b) public {
		// Removed due to Spacer's nondeterminism.
		//a[1][1] = 512;
		if (b)
			delete a;
		else
			delete a[1];
		init();
		assert(a[1][0] == 0);
		assert(a[0][0] == 0);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (315-335): CHC: Assertion violation might happen here.
// Warning 4661: (315-335): BMC: Assertion violation happens here.
