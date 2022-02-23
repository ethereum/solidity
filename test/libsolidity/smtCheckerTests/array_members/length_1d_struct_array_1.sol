contract C {
	struct S {
		uint[] arr;
	}
	S s1;
	S s2;
	function f() public view {
		assert(s1.arr.length == s2.arr.length);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(((s1.arr.length + ((- 1) * s2.arr.length)) >= 0) && ((s1.arr.length + ((- 1) * s2.arr.length)) <= 0))\n
