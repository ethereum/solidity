contract C {
	bytes16[][] a;
	constructor() {
		a.push();
		a.push();
		a.push();
		a.push();
		a.push();
		a.push();
		a[2].push();
		a[2].push();
		a[2].push();
		a[3].push();
		a[3].push();
		a[3].push();
		a[3].push();
		a[3].push();
	}
	function g() internal view returns (bytes16[] storage) {
		return a[2];
	}
	function h() internal view returns (bytes16) {
		return a[2][2];
	}
	function f() external view {
		// Reports oob because of aliasing.
		g()[3][4];
		h()[5];
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6368: (374-381): CHC: Out of bounds access might happen here.
// Warning 6368: (456-462): CHC: Out of bounds access happens here.
// Info 1180: Contract invariant(s) for :C:\n!(a.length <= 4)\n
