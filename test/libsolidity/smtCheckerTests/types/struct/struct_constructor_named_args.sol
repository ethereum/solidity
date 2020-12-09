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
		S memory inner = S({x: 43});
		T memory outer = T({y: 512, s: inner});
		assert(outer.y == 512);
		assert(outer.s.x == 43);
		assert(outer.s.x == 42);
	}
}
// ----
// Warning 6328: (265-288): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ntest()
