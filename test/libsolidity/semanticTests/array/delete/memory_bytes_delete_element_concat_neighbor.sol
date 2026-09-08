// Companion to memory_bytes_delete_element_neighbor.sol for byte arrays produced by `bytes.concat`
// and `string.concat`. Under evmasm, both advance the free memory pointer by exactly the payload length and
// does not round up to a 32-byte boundary, so the next object starts right after the last byte,
// i.e. E = D+L and P = 0 regardless of L. Without padding to absorb it, a full-word write from
// index i reaches the next object for every i >= L-31, and the number of spilled bytes is i+32-L.
contract C {
    struct S { uint256 x; }

    // L=1, so a word written from index 0 covers the top 31 bytes of s.x
    function concatSingleByte() external pure returns (uint256) {
        bytes memory b = bytes.concat(bytes1(0x01));
        S memory s = S(type(uint256).max);
        delete b[0];
        return s.x;
    }

    // L=33, i=2=L-31, so exactly the top byte of s.x is in reach
    function concatFarFromEnd() external pure returns (uint256) {
        bytes memory b = bytes.concat(new bytes(32), bytes1(0x01));
        S memory s = S(type(uint256).max);
        delete b[2];
        return s.x;
    }

    // L=6, i=4, so 30 bytes of the neighbor's length word are in reach; 300 fits into the remaining two
    function concatDynArrayNeighborSurvives() external pure returns (uint256) {
        bytes memory b = bytes.concat(new bytes(5), bytes1(0x01));
        uint256[] memory a = new uint256[](300);
        delete b[4];
        return a.length;
    }

    // L=6, i=5, so 31 bytes are in reach and only the low byte of the length (300 = 0x012c) survives
    function concatDynArrayNeighborTruncates() external pure returns (uint256) {
        bytes memory b = bytes.concat(new bytes(5), bytes1(0x01));
        uint256[] memory a = new uint256[](300);
        delete b[5];
        return a.length;
    }

    // The payload itself: clearing index 1 of a 6-byte concat result must leave bytes 2..5 intact
    function concatPayload() external pure returns (bytes memory) {
        bytes memory b = bytes.concat(hex"0102", hex"03040506");
        delete b[1];
        return b;
    }
    // `string.concat` uses the same allocation, and `bytes(s)` aliases the string's memory. L=1.
    function stringConcatSingleChar() external pure returns (uint256) {
        string memory s = string.concat("a");
        S memory t = S(type(uint256).max);
        delete bytes(s)[0];
        return t.x;
    }

    // L=6, i=5, so 31 bytes reach into the neighbour's length word
    function stringConcatDynArrayNeighborTruncates() external pure returns (uint256) {
        string memory s = string.concat("abc", "def");
        uint256[] memory a = new uint256[](300);
        delete bytes(s)[5];
        return a.length;
    }

    function stringConcatPayload() external pure returns (string memory) {
        string memory s = string.concat("ab", "cdef");
        delete bytes(s)[1];
        return s;
    }
}
// ----
// concatSingleByte() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// concatFarFromEnd() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// concatDynArrayNeighborSurvives() -> 300
// concatDynArrayNeighborTruncates() -> 300
// concatPayload() -> 0x20, 6, left(0x010003040506)
// stringConcatSingleChar() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// stringConcatDynArrayNeighborTruncates() -> 300
// stringConcatPayload() -> 0x20, 6, "a\x00cdef"
