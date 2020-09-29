==== Source: s1.sol ====
import {f as g, g as h} from "s2.sol";
function f() pure returns (uint) { return h() - g(); }
==== Source: s2.sol ====
import {f as h} from "s1.sol";
function f() pure returns (uint) { return 2; }
function g() pure returns (uint) { return 4; }
==== Source: s3.sol ====
import "s1.sol";
