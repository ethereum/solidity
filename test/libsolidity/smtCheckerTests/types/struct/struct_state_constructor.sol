pragma experimental SMTChecker;

contract C {

	struct S {
		uint x;
	}

	S s = S(42);

	function test() view public {
		assert(s.x == 42);
	}
}
// ----
