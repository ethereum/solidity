contract C {
    function f(int32 a, int32 b) public returns (int32) {
        return a >> b;
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(int32,int32): 0xffffff99, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff99
// f(int32,int32): 0xffffff99, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffcc
// f(int32,int32): 0xffffff99, 0x02 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe6
// f(int32,int32): 0xffffff99, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff9
// f(int32,int32): 0xffffff99, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
