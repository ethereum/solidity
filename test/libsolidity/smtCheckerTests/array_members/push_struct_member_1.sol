pragma experimental SMTChecker;
contract C {
	struct S {
		int[] b;
	}
	S s;
	struct T {
		S s;
	}
	T t;
	function f() public {
		s.b.push();
		t.s.b.push();
	}
}

// ----
