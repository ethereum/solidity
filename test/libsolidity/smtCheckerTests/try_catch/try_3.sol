contract C {
	int x;

	function g(int y) public pure returns (int) {
		return y;
	}

	function postinc() internal returns (int) {
		x += 1;
		return x;
	}

	function f() public {
		x = 0;
		try this.g(postinc()) {
			assert(x == 1); // should hold
		} catch (bytes memory s) {
			assert(x == 0); // should fail - state is reverted to the state after postinc(), but before the call to this.g()
		}
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 5667: (259-273): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (280-294): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\ns = []\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f()\n    C.postinc() -- internal call
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
