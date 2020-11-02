pragma experimental ABIEncoderV2;


contract C {
    function f(int8 a, uint8 b) public returns (int256) {
        assembly {
            a := 0xfffffff0
        }
        // Higher bits should be signextended before the shift
        return a >> b;
    }

    function g(int8 a, uint8 b) public returns (int256) {
        assembly {
            a := 0xf0
        }
        // Higher bits should be signextended before the shift
        return a >> b;
    }
}
// ====
// compileViaYul: also
// ----
// f(int8,uint8): 0x00, 0x03 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe
// f(int8,uint8): 0x00, 0x04 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,uint8): 0x00, 0xff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,uint8): 0x00, 0x1003 -> FAILURE
// f(int8,uint8): 0x00, 0x1004 -> FAILURE
// g(int8,uint8): 0x00, 0x03 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe
// g(int8,uint8): 0x00, 0x04 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// g(int8,uint8): 0x00, 0xff -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// g(int8,uint8): 0x00, 0x1003 -> FAILURE
// g(int8,uint8): 0x00, 0x1004 -> FAILURE
