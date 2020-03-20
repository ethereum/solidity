contract C {
    function f(uint8 a, uint8 b) public returns (uint256) {
        assembly {
            a := 0xffffffff
        }
        // Higher bits should be cleared before the shift
        return a >> b;
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(uint8,uint8): 0x00, 0x04 -> 0x0f
// f(uint8,uint8): 0x00, 0x1004 -> 0x0f
