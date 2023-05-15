==== Source: s1.sol ====
error E(uint);
==== Source: s2.sol ====
import { E as Panic } from "s1.sol";
contract C {
    error E(uint);
    function a() public pure {
        revert Panic(1);
    }
    function b() public pure {
        revert E(1);
    }
}
// ----
// a() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001"
// b() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001"
