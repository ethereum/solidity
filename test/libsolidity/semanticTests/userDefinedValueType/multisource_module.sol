==== Source: s1.sol ====
type MyInt is int;
==== Source: s2.sol ====
import "s1.sol" as M;
contract C {
  function f(int x) public pure returns (M.MyInt) { return M.MyInt.wrap(x); }
  function g(M.MyInt x) public pure returns (int) { return M.MyInt.unwrap(x); }
}
// ====
// compileViaYul: also
// ----
// f(int256): 5 -> 5
// g(int256): 1 -> 1
