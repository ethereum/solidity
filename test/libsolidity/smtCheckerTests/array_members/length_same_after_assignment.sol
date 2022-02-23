contract C {
	uint[] arr;
	constructor() {
		arr.push();
		arr.push();
		arr.push();
		arr.push();
	}
	function f() public view {
		uint[] memory arr2 = arr;
		arr2[2] = 3;
		assert(arr.length == arr2.length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n!(arr.length <= 2)\n
