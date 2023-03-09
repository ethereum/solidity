==== Source: s1.sol ====
type MyInt is int;
==== Source: s2.sol ====
import "s1.sol" as M;
contract C {
	function f(int x) public pure returns (M.MyInt) { return M.MyInt.wrap(x); }
	function g(M.MyInt x) public pure returns (int) { return M.MyInt.unwrap(x); }

	function h() public pure {
		assert(M.MyInt.unwrap(f(5)) == 5);
		assert(M.MyInt.unwrap(f(5)) == 6); // should fail
		assert(g(M.MyInt.wrap(1)) == 1);
		assert(g(M.MyInt.wrap(1)) == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreOS: macos
// ----
// Warning 6328: (s2.sol:259-292): CHC: Assertion violation happens here.
// Warning 6328: (s2.sol:346-377): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
