contract C {
	struct S {
		uint[][] arr;
	}
	S s1;
	S s2;
	constructor() {
		s1.arr.push();
		s2.arr.push();
		s1.arr[0].push();
		s1.arr[0].push();
		s1.arr[0].push();
		s2.arr[0].push();
		s2.arr[0].push();
		s2.arr[0].push();
	}
	function f() public view {
		assert(s1.arr[0].length == s2.arr[0].length);
	}
}
// ====
// SMTEngine: all
// ----
