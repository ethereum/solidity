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
// Warning 6368: (456-462): CHC: Out of bounds access happens here.
// Info 1391: CHC: 13 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
