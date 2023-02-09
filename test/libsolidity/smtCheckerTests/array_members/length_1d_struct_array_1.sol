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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
