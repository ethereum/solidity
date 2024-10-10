abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;

	function inc() public {
		require(x < 10);
		++x;
	}

	function f() public {
		d.d();
		assert(x < 5);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (175-188): CHC: Assertion violation happens here.\nCounterexample:\nx = 5, d = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.inc()\nState: x = 1, d = 0\nC.inc()\nState: x = 2, d = 0\nC.inc()\nState: x = 3, d = 0\nC.inc()\nState: x = 4, d = 0\nC.f()\n    d.d() -- untrusted external call, synthesized as:\n        C.inc() -- reentrant call
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
