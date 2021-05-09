abstract contract D {
	function d() external virtual returns (uint x, bool b);
}

contract C {

	int x;
	D d;

	function f() public {
		x = 0;
		try d.d() returns (uint x, bool c) {
			assert(x == 0); // should fail, x is the local variable shadowing the state variable
			assert(!c); // should fail, c can be anything
		} catch {
			assert(x == 0); // should hold, x is the state variable
			assert(x == 1); // should fail
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2519: (164-170): This declaration shadows an existing declaration.
// Warning 6328: (185-199): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, d = 0\nx = 1\nc = false\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.f()\n    d.d() -- untrusted external call
// Warning 6328: (273-283): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, d = 0\nx = 1\nc = true\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.f()\n    d.d() -- untrusted external call
// Warning 6328: (393-407): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, d = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.f()
