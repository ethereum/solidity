==== Source: A.sol ====
contract A {
	uint x;
	function f(uint _x) public {
		x = _x;
	}
}
==== Source: B.sol ====
import "A.sol";
pragma experimental SMTChecker;
contract B is A {
	function g(uint _x) public view {
		assert(_x > x);
	}
}
==== Source: C.sol ====
import "A.sol";
pragma experimental SMTChecker;
contract C is A {
	function h(uint _x) public view {
		assert(_x < x);
	}
}
// ----
// Warning 6328: (B.sol:103-117): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\nTransaction trace:\nB.constructor()\nState: x = 0\nB.g(0)
// Warning 6328: (C.sol:103-117): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.h(0)
