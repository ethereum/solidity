pragma experimental SMTChecker;

abstract contract D {
	function d() external virtual;
}

contract C {

	int x;
	D d;

	function set(int n) public {
		x = n;
	}

	function f() public {
		x = 0;
		try d.d() {
			assert(x == 0); // should fail, x can be anything here
		} catch {
			assert(x == 0); // should hold, all changes to x has been reverted
			assert(x == 1); // should fail
		}
	}
}
// ----
// Warning 6328: (211-225): CHC: Assertion violation happens here.\nCounterexample:\nx = (- 1), d = 0\n\n\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.f()
// Warning 6328: (351-365): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, d = 0\n\n\n\nTransaction trace:\nC.constructor()\nState: x = 0, d = 0\nC.f()
