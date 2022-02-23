==== Source: s1.sol ====
type MyInt is int;
==== Source: s2.sol ====
import "s1.sol" as M;
contract C {
  function f(int x) public pure returns (MyInt) { return MyInt.wrap(x); }
}
// ----
// DeclarationError 7920: (s2.sol:76-81): Identifier not found or not unique.
