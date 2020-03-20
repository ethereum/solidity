pragma experimental ABIEncoderV2;


contract C {
    function f(int32 a, int32 b) public returns (int32) {
        return a >> b;
    }
}
// ----
// f(int32,int32): 0xffffff99, 0x00 -> FAILURE
// f(int32,int32): 0xffffff99, 0x01 -> FAILURE
// f(int32,int32): 0xffffff99, 0x02 -> FAILURE
// f(int32,int32): 0xffffff99, 0x04 -> FAILURE
// f(int32,int32): 0xffffff99, 0x08 -> FAILURE
