==== Source: s1.sol ====
uint constant a = 89;
==== Source: s2.sol ====
uint constant a = 88;

==== Source: s3.sol ====
import "s1.sol" as M;
import "s2.sol" as N;

contract C {
	function f() internal pure returns (uint, uint) {
		return (M.a, N.a);
	}
	function p() public pure {
		(uint x, uint y) = f();
		assert(x == 89); // should hold
		assert(x == 88); // should fail
		assert(y == 88); // should hold
		assert(y == 89); // should fail
	}
}
// ====
// SMTEngine: chc
// ----
// Warning 6328: (s3.sol:223-238): CHC: Assertion violation happens here.
// Warning 6328: (s3.sol:291-306): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
