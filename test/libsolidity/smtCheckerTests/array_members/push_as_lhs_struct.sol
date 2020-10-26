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
		s.b.push() = t.s.b.push();
		assert(s.b[s.b.length -1] == t.s.b[t.s.b.length - 1]);
	}
}
