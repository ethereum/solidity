contract C {
    function f(int16 a, int16 b) public returns (int16) {
        return a >> b;
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(int16,int16): 0xff99, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff99
// f(int16,int16): 0xff99, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffcc
// f(int16,int16): 0xff99, 0x02 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe6
// f(int16,int16): 0xff99, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff9
// f(int16,int16): 0xff99, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
