pragma experimental ABIEncoderV2;
contract C {
    function f(string[] calldata a) external returns(uint, uint, uint, string memory) {
        string memory s1 = a[0];
        bytes memory m1 = bytes(s1);
        return (a.length, m1.length, uint8(m1[0]), s1);
    }
}

// ----
// f(string[]): 0x20, 1, 0x20, 2, bytes{'a', 'b'} + bytes(30, 0) -> 1, 2, 'a', 0x80, 2, bytes{'a', 'b'} + bytes(30, 0
// f(string[]):"32, 1, 32, 2, [61,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "1, 2, a, 128, 2, [61,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]"
