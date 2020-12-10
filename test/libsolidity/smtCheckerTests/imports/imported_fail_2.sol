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
import "B.sol";
pragma experimental SMTChecker;
contract C is B {
	function h(uint _x) public view {
		assert(_x < x);
	}
}
// ----
// Warning 6328: (B.sol:103-117): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(0)
// Warning 6328: (B.sol:103-117): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ng(0)
// Warning 6328: (C.sol:103-117): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nh(0)
