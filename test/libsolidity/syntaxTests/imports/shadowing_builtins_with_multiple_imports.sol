==== Source: B.sol ====
contract msg {} contract block{}
==== Source: b ====
import {msg, block} from "B.sol";
contract C {
}
// ----
// Warning 2319: (B.sol:0-15): This declaration shadows a builtin symbol.
// Warning 2319: (B.sol:16-32): This declaration shadows a builtin symbol.
// Warning 2319: (b:8-11): This declaration shadows a builtin symbol.
// Warning 2319: (b:13-18): This declaration shadows a builtin symbol.
