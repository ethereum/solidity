// `delete b[i]` on a memory byte-array element must clear exactly one byte and leave the memory
// after the buffer untouched. Prior to the MemoryByteArrayElementDeleteClearsWholeWord fix, the
// compiler wrote a full 32-byte word under evmasm.
// For a delete near the end of the buffer, that word spilled past it into the next object.
//
// Layout: a `bytes` array of length `L` is rounded up to `R = 32*ceil(L/32)` bytes, occupying `[D, D+R)`
// with `P = R-L` padding bytes. `E = D+R` is the first byte past it and, under the bump allocator,
// the start of the next object. If `delete b[i]` writes a word over [D+i, D+i+31], it reaches
// past E into that object whenever i >= R-31.
//
// The spill always runs from E upward, i.e. over the most significant bytes of the next word:
// - a struct or fixed-size array holds a value there and loses its top bytes;
// - a dynamic array holds its length there, right-aligned, so the low bytes survive and it is
//   truncated only once the element count is large enough to reach the overwritten bytes.
contract C {
    struct S { uint256 x; }

    // L=32, P=0
    function structNeighborLastElement() external pure returns (uint256) {
        bytes memory b = new bytes(32);
        S memory s = S(type(uint256).max);
        delete b[31];
        return s.x;
    }

    // L=32, P=0
    function structNeighborOneShort() external pure returns (uint256) {
        bytes memory b = new bytes(32);
        S memory s = S(type(uint256).max);
        delete b[30];
        return s.x;
    }

    // A dynamic array neighbour with a small element count keeps all of its length in the low,
    // unreachable byte E+31
    function dynArrayNeighborSurvivesSmallCount() external pure returns (uint256) {
        bytes memory b = new bytes(32);
        uint256[] memory a = new uint256[](5);
        delete b[31];
        return a.length;
    }

    // With 256 elements the length needs byte E+30
    function dynArrayNeighborTruncatesLargeCount() external pure returns (uint256) {
        bytes memory b = new bytes(32);
        uint256[] memory a = new uint256[](256);
        delete b[31];
        return a.length;
    }

    // L=1, so P=31
    function paddingImmunity() external pure returns (uint256) {
        bytes memory b = new bytes(1);
        S memory s = S(type(uint256).max);
        delete b[0];
        return s.x;
    }
}
// ====
// compileViaYul: false
// ----
// structNeighborLastElement() -> 0xff
// structNeighborOneShort() -> 0xffff
// dynArrayNeighborSurvivesSmallCount() -> 5
// dynArrayNeighborTruncatesLargeCount() -> 0
// paddingImmunity() -> -1
