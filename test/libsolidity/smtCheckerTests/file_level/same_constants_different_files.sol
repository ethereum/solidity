==== Source: s1.sol ====


uint constant a = 89;

function fre() pure returns (uint) {
	return a;
}

==== Source: s2.sol ====

import {a as b, fre} from "s1.sol";
import "s1.sol" as M;

uint256 constant a = 13;

contract C {
	function f() internal pure returns (uint, uint, uint, uint) {
		return (a, fre(), M.a, b);
	}
	function p() public pure {
		(uint x, uint y, uint z, uint t) = f();
		assert(x == 13); // should hold
		assert(y == 89); // should hold
		assert(z == 89); // should hold but the SMTChecker does not implement module access
		assert(t == 89); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 7650: (s2.sol:182-185): Assertion checker does not yet support this expression.
// Warning 8364: (s2.sol:182-183): Assertion checker does not yet implement type module "s1.sol"
// Warning 6328: (s2.sol:334-349): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 13\ny = 89\nz = 90\nt = 89\n\nTransaction trace:\nC.constructor()\nC.p()\n    C.f() -- internal call\n        s1.sol:fre() -- internal call
// Warning 7650: (s2.sol:182-185): Assertion checker does not yet support this expression.
// Warning 8364: (s2.sol:182-183): Assertion checker does not yet implement type module "s1.sol"
// Warning 7650: (s2.sol:182-185): Assertion checker does not yet support this expression.
