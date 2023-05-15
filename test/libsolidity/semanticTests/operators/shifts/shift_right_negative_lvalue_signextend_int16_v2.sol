pragma abicoder               v2;


contract C {
    function f(int16 a, uint16 b) public returns (int16) {
        return a >> b;
    }
}
// ----
// f(int16,uint16): 0xff99, 0x00 -> FAILURE
// f(int16,uint16): 0xff99, 0x01 -> FAILURE
// f(int16,uint16): 0xff99, 0x02 -> FAILURE
// f(int16,uint16): 0xff99, 0x04 -> FAILURE
// f(int16,uint16): 0xff99, 0x08 -> FAILURE
