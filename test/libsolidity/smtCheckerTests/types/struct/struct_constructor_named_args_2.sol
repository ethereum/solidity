pragma experimental SMTChecker;

contract C {

	struct S {
		uint x;
		uint y;
		uint z;
	}

	function test() pure public {
		S memory s = S({z: 1, y: 2, x: 3});
		assert(s.x == 3);
		assert(s.y == 2);
		assert(s.z == 1);
		assert(s.x == 0 || s.y == 0 || s.z == 0); // should fail
	}
}
// ----
// Warning 6328: (224-264): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ntest()
