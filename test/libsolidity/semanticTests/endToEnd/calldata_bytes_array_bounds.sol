pragma experimental ABIEncoderV2;
contract C {
    function f(bytes[] calldata a, uint256 i) external returns(uint) {
        return uint8(a[0][i]);
    }
}

// ----
// f(bytes[],uint256): 0x40, 0, 1, 0x20, 2, bytes{'a', 'b'} + bytes(30, 0) -> 'a'
// f(bytes[],uint256):"64, 0, 1, 32, 2, [61,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "a"
// f(bytes[],uint256): 0x40, 1, 1, 0x20, 2, bytes{'a', 'b'} + bytes(30, 0) -> 'b'
// f(bytes[],uint256):"64, 1, 1, 32, 2, [61,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> "b"
// f(bytes[],uint256): 0x40, 2, 1, 0x20, 2, bytes{'a', 'b'} + bytes(30, 0) -> 
// f(bytes[],uint256):"64, 2, 1, 32, 2, [61,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]" -> ""
