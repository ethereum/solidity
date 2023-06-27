==== Source: a.sol ====
contract A {
	uint x;
}
==== Source: b.sol ====
import "a.sol";
contract B is A {
	function g() public view { assert(x > x); }
}
==== Source: c.sol ====
import "b.sol";
contract C is B {
	function h(uint x) public pure { assert(x < x); }
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6328: (b.sol:62-75): CHC: Assertion violation happens here.
// Warning 6328: (c.sol:68-81): CHC: Assertion violation happens here.
