==== Source: A.sol ====
contract A {
	uint x;
	function f(uint _x) public {
		x = _x;
	}
}
==== Source: B.sol ====
import "A.sol";
contract B is A {
	function g(uint _x) public view {
		assert(_x > x);
	}
}
==== Source: C.sol ====
import "B.sol";
contract C is B {
	function h(uint _x) public view {
		assert(_x < x);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (B.sol:71-85): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\nTransaction trace:\nB.constructor()\nState: x = 0\nB.g(0)
// Warning 6328: (C.sol:71-85): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n_x = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.h(0)
