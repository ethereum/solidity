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
// SMTIgnoreOS: macos
// ----
// Info 1180: Contract invariant(s) for :C:\n!(s1.arr.length <= 0)\n!(s2.arr.length <= 0)\n(((s1.arr[0].length + ((- 1) * s2.arr[0].length)) <= 0) && ((s2.arr[0].length + ((- 1) * s1.arr[0].length)) <= 0))\n
