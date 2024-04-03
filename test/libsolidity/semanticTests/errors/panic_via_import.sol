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
    function encode_a() public pure returns (bytes memory) {
        return abi.encodeError(Panic, (1));
    }
    function encode_b() public pure returns (bytes memory) {
        return abi.encodeError(E, (1));
    }
}
// ----
// a() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001"
// b() -> FAILURE, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001"
// encode_a() -> 0x20, 0x24, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001", hex"00000000000000000000000000000000000000000000000000000000"
// encode_b() -> 0x20, 0x24, hex"002ff067", hex"0000000000000000000000000000000000000000000000000000000000000001", hex"00000000000000000000000000000000000000000000000000000000"
