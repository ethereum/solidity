pragma abicoder v1;
contract C {
    function f(uint16 a, int16 b, address c, bytes3 d, bool e)
            public pure returns (uint v, uint w, uint x, uint y, uint z) {
        assembly { v := a  w := b x := c y := d z := e}
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(uint16,int16,address,bytes3,bool): 1, 2, 3, "a", true -> 1, 2, 3, "a", true
// f(uint16,int16,address,bytes3,bool): 0xffffff, 0x1ffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "abcd", 1 -> 0xffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffff, "abc", true
// f(uint16,int16,address,bytes3,bool): 0xffffff, 0, 0, "bcd", 1 -> 0xffff, 0, 0, "bcd", true
// f(uint16,int16,address,bytes3,bool): 0, 0x1ffff, 0, "ab", 1 -> 0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0, "ab", true
// f(uint16,int16,address,bytes3,bool): 0, 0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, "ad", 1 -> 0, 0, 0xffffffffffffffffffffffffffffffffffffffff, "ad", true
// f(uint16,int16,address,bytes3,bool): 0, 0, 0, "abcd", 1 -> 0, 0, 0, "abc", true
// f(uint16,int16,address,bytes3,bool): 0, 0, 0, "abc", 2 -> 0, 0, 0, "abc", true
