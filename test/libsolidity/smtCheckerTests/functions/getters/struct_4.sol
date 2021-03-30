pragma experimental SMTChecker;

contract D {
}

contract C {
	struct S {
		D d;
		function () external returns (uint) f;
	}

	S public s;

	function test() public view {
		(D d, function () external returns (uint) f) = this.s();
		assert(d == s.d); // should hold
		assert(address(d) == address(this)); // should fail
	}
}
// ----
// Warning 2072: (179-216): Unused local variable.
// Warning 6328: (267-302): CHC: Assertion violation happens here.\nCounterexample:\ns = {d: 0, f: 0}\nd = 0\nf = 0\n\nTransaction trace:\nC.constructor()\nState: s = {d: 0, f: 0}\nC.test()
