contract C {
	uint[][] arr;

	constructor() {
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
		arr.push();
	}

	function f() public {
		uint x = arr[2].length;
		uint y = arr[3].length;
		uint z = arr.length;
		uint t = arr[5].length;
		arr[5] = arr[8];
		assert(arr[2].length == x);
		assert(arr[3].length == y);
		assert(arr.length == z);
		assert(arr[5].length == t);
	}
}
// ====
// SMTEngine: all
// ----
