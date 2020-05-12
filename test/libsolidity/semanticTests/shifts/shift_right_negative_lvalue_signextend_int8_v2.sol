pragma experimental ABIEncoderV2;


contract C {
    function f(int8 a, int8 b) public returns (int8) {
        return a >> b;
    }
}
// ====
// compileViaYul: also
// ----
// f(int8,int8): 0x99, 0x00 -> FAILURE
// f(int8,int8): 0x99, 0x01 -> FAILURE
// f(int8,int8): 0x99, 0x02 -> FAILURE
// f(int8,int8): 0x99, 0x04 -> FAILURE
// f(int8,int8): 0x99, 0x08 -> FAILURE
