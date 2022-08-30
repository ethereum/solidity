==== Source: A ====
import "s1.sol" as M;
function f(uint _x) pure {
	assert(_x > 0);
}
contract D {
	function g(uint _y) public pure {
		M.f(200); // should hold
		M.f(_y); // should fail
		f(10); // should hold
		f(_y); // should fail
	}
}
==== Source: s1.sol ====
function f(uint _x) pure {
	assert(_x > 100);
}
// ====
// SMTEngine: all
// ----
// Warning 1218: (A:50-64): CHC: Error trying to invoke SMT solver.
// Warning 1218: (s1.sol:28-44): CHC: Error trying to invoke SMT solver.
// Warning 6328: (A:50-64): CHC: Assertion violation might happen here.
// Warning 6328: (s1.sol:28-44): CHC: Assertion violation might happen here.
// Warning 4661: (s1.sol:28-44): BMC: Assertion violation happens here.
// Warning 4661: (A:50-64): BMC: Assertion violation happens here.
