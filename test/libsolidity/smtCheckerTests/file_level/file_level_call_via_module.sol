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
// Warning 6328: (b.sol:208-222): CHC: Assertion violation happens here.
// Warning 6328: (b.sol:274-288): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
