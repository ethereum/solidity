abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;
	function f() public {
		if (x < 10)
			++x;
	}
	function g() public {
		d.d();
		assert(x < 10);
	}
}
// ====
// SMTEngine: chc
// SMTTargets: assert
// SMTIgnoreCex: yes
// SMTIgnoreOS: linux
// ----
// Warning 6328: (167-181): CHC: Assertion violation happens here.\nCounterexample:\nx = 10, d = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.f()\nState: x = 1, d = 0\nC.f()\nState: x = 2, d = 0\nC.f()\nState: x = 3, d = 0\nC.f()\nState: x = 4, d = 0\nC.f()\nState: x = 5, d = 0\nC.f()\nState: x = 6, d = 0\nC.g()\n    d.d() -- untrusted external call, synthesized as:\n        C.f() -- reentrant call\n        C.f() -- reentrant call\nState: x = 8, d = 0\nC.g()\n    d.d() -- untrusted external call, synthesized as:\n        C.f() -- reentrant call\n        C.f() -- reentrant call
