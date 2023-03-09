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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (291-317): CHC: Assertion violation happens here.
// Warning 6328: (321-347): CHC: Assertion violation happens here.
// Warning 6328: (351-374): CHC: Assertion violation happens here.
// Info 1391: CHC: 10 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
