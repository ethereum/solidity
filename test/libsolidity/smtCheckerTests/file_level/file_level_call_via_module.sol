==== Source: a.sol ====
function f(uint) pure returns (uint) { return 7; }
function f(bytes memory x) pure returns (uint) { return x.length; }
==== Source: b.sol ====
import "a.sol" as M;
contract C {
	function f() internal pure returns (uint, uint) {
		return (M.f(2), M.f("abc"));
	}
	function p() public pure {
		(uint a, uint b) = f();
		assert(a == 7); // should hold
		assert(a == 9); // should fail
		assert(b == 3); // should hold
		assert(b == 5); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8364: (b.sol:95-96): Assertion checker does not yet implement type module "a.sol"
// Warning 8364: (b.sol:103-104): Assertion checker does not yet implement type module "a.sol"
// Warning 6328: (b.sol:208-222): CHC: Assertion violation happens here.\nCounterexample:\n\na = 7\nb = 3\n\nTransaction trace:\nC.constructor()\nC.p()\n    C.f() -- internal call\n        a.sol:f(2) -- internal call\n        a.sol:f([97, 98, 99]) -- internal call
// Warning 6328: (b.sol:274-288): CHC: Assertion violation happens here.\nCounterexample:\n\na = 7\nb = 3\n\nTransaction trace:\nC.constructor()\nC.p()\n    C.f() -- internal call\n        a.sol:f(2) -- internal call\n        a.sol:f([97, 98, 99]) -- internal call
// Warning 8364: (b.sol:95-96): Assertion checker does not yet implement type module "a.sol"
// Warning 8364: (b.sol:103-104): Assertion checker does not yet implement type module "a.sol"
