pragma experimental SMTChecker;

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
		assert(arr[2].length != x);
		assert(arr[3].length != y);
		assert(arr.length != z);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (324-350): CHC: Assertion violation happens here.
// Warning 6328: (354-380): CHC: Assertion violation happens here.
// Warning 6328: (384-407): CHC: Assertion violation happens here.
