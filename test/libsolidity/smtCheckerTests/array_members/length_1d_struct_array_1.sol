pragma experimental SMTChecker;

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
// ----
