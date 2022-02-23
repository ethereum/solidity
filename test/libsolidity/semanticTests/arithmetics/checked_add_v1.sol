pragma abicoder v1;
contract C {
    // Input is still not checked - this needs ABIEncoderV2!
    function f(uint16 a, uint16 b) public returns (uint16) {
        return a + b;
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(uint16,uint16): 65534, 0 -> 0xfffe
// f(uint16,uint16): 65536, 0 -> 0x00
// f(uint16,uint16): 65535, 0 -> 0xffff
// f(uint16,uint16): 65535, 1 -> FAILURE, hex"4e487b71", 0x11
