contract C {
	uint[][] arr;
	constructor() {
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr[2].push();
		arr[2].push();
		arr[2].push();
		arr[2].push();
	}
	function f() public {
		uint x = arr[2].length;
		uint y = arr[3].length;
		uint z = arr.length;
		arr[2][3] = 444;
		assert(arr[2].length == x);
		assert(arr[3].length == y);
		assert(arr.length == z);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 13 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
