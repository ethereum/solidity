contract C {
	struct S {
		uint x;
		uint[] a;
	}
	function s() internal pure returns (S memory s1) {
		s1.x = 42;
		s1.a = new uint[](5);
		s1.a[2] = 43;
	}
	function f() public pure {
		S memory s2 = s();
		assert(s2.x == 42);
		assert(s2.a[2] == 43);
		assert(s2.a[3] == 43);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (256-277): CHC: Assertion violation happens here.\nCounterexample:\n\ns2 = {x: 42, a: [0, 0, 43, 0, 0]}\n\nTransaction trace:\nC.constructor()\nC.f()\n    C.s() -- internal call
