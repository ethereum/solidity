pragma abicoder v1;
contract C {
    function f(int16 a, uint16 b) public returns (int16) {
        return a >> b;
    }
}
// ====
// ABIEncoderV1Only: true
// compileViaYul: false
// ----
// f(int16,uint16): 0xff99, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff99
// f(int16,uint16): 0xff99, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffcc
// f(int16,uint16): 0xff99, 0x02 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe6
// f(int16,uint16): 0xff99, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff9
// f(int16,uint16): 0xff99, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
