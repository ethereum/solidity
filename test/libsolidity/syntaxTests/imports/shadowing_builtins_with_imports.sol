==== Source: B.sol ====
contract X {}
==== Source: b ====
import * as msg from "B.sol";
contract C {
}
// ----
// Warning: (b:0-29): This declaration shadows a builtin symbol.
