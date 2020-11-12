pragma experimental SMTChecker;

contract C {

	struct S {
		uint x;
	}

	struct T {
		S s;
		uint y;
	}

	function test() pure public {
		assert(T(S(42), 1).s.x == 42);
	}
}
// ----
