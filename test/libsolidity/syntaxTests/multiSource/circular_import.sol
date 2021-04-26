==== Source: s1.sol ====
import {f as g} from "s2.sol";
function f() pure returns (uint) { return 1; }
==== Source: s2.sol ====
import {f as g} from "s1.sol";
function f() pure returns (uint) { return 2; }
// ----
