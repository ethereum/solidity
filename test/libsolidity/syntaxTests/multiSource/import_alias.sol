==== Source: s1.sol ====
int constant a = 2;
==== Source: s2.sol ====
import {a as e} from "s1.sol";
import "s2.sol" as M;
contract C {
  function f() public pure returns (int) { return M.e; }
}
// ----
