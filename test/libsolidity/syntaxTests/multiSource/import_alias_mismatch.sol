==== Source: s1.sol ====
int constant a = 2;
==== Source: s2.sol ====
import {a as e} from "s1.sol";
import "s2.sol" as M;
contract C {
  function f() public pure returns (int) { return M.a; }
}
// ----
// TypeError 9582: (s2.sol:116-119): Member "a" not found or not visible after argument-dependent lookup in module "s2.sol".
