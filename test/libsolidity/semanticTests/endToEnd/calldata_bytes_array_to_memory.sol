pragma experimental ABIEncoderV2;
contract C {
    function f(bytes[] calldata a) external returns(uint, uint, bytes memory) {
        bytes memory m = a[0];
        return (a.length, m.length, m);
    }
}

// ----
// f(bytes[]): 0x20, 1, 0x20, 2, "ab", 30, 0 -> 1, 2, 0x60, 2, "ab"
// f(bytes[]): 0x20, 1, 0x20, 32, 32, "x", 30, 0 -> 1, 32, 0x60, 32, 32
// f(bytes[]): 0x20, 1, 0x20, 0x20, "amx" -> 1, 0x20, 0x60, 0x20, "amx"
