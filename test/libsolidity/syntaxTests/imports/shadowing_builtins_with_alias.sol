==== Source: B.sol ====
contract C {}
==== Source: b ====
import {C as msg} from "B.sol";
// ----
// Warning: (b:13-16): This declaration shadows a builtin symbol.
