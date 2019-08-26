==== Source: B.sol ====
contract C {}
==== Source: b ====
import {C as msg} from "B.sol";
// ----
// Warning: (B.sol:0-13): This declaration shadows a builtin symbol.
