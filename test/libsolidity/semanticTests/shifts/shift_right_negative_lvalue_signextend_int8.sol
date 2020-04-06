contract C {
    function f(int8 a, int8 b) public returns (int8) {
        return a >> b;
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(int8,int8): 0x99, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff99
// f(int8,int8): 0x99, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffcc
// f(int8,int8): 0x99, 0x02 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe6
// f(int8,int8): 0x99, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff9
// f(int8,int8): 0x99, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
