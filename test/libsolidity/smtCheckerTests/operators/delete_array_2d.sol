contract C
{
	uint[][] a;
	constructor() {
		a.push(); a.push(); a.push();
		a[2].push();
		a[2].push();
		a[2].push();
		a[2].push();
	}
	function f() public {
		require(a[2][3] == 4);
		delete a;
		assert(a[2][3] == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(true && !(a.length <= 2))\n(true && !(a[2].length <= 3))\n
