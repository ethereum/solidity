pragma abicoder               v2;

contract C {
	uint[][] arr;
	uint[][] arr2;
	constructor() {
		arr.push();
		arr.push();
		arr2.push();
		arr2.push();
	}
	function f() public view {
		assert(arr2[0].length == arr[0].length);
		assert(arr2.length == arr.length);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Info 1180: Contract invariant(s) for :C:\n!(arr.length <= 0)\n!(arr2.length <= 0)\n(((arr.length + ((- 1) * arr2.length)) <= 0) && ((arr2.length + ((- 1) * arr.length)) <= 0))\n(((arr2[0].length + ((- 1) * arr[0].length)) >= 0) && ((arr2[0].length + ((- 1) * arr[0].length)) <= 0))\n
