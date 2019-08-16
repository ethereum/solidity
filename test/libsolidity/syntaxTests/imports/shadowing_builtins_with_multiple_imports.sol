==== Source: B.sol ====
contract msg {} contract block{}
==== Source: b ====
import {msg, block} from "B.sol";
contract C {
}
// ----
// Warning: (B.sol:0-15): This declaration shadows a builtin symbol.
// Warning: (B.sol:16-32): This declaration shadows a builtin symbol.
// Warning: (B.sol:0-15): This declaration shadows a builtin symbol.
// Warning: (B.sol:16-32): This declaration shadows a builtin symbol.
