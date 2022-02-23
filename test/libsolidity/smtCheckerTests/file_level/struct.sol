struct S {
	uint x;
	uint[] a;
}

function allocate(uint _x, uint _f) pure returns (S memory) {
	S memory s;
	s.x = _x;
	s.a = new uint[](1);
	s.a[0] = _f;
	return s;
}

contract C {
	function f() public pure {
		S memory s = allocate(2, 1);
		assert(s.x == 2); // should hold
		assert(s.a[0] == 1); // should hold
		assert(s.x == 3); // should fail
		assert(s.a[0] == 4); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (317-333): CHC: Assertion violation happens here.\nCounterexample:\n\ns = {x: 2, a: [1]}\n\nTransaction trace:\nC.constructor()\nC.f()\n    allocate(2, 1) -- internal call
// Warning 6328: (352-371): CHC: Assertion violation happens here.\nCounterexample:\n\ns = {x: 2, a: [1]}\n\nTransaction trace:\nC.constructor()\nC.f()\n    allocate(2, 1) -- internal call
