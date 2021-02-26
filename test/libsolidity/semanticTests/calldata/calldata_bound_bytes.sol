pragma abicoder v2;

library L {
    function reverse(bytes calldata _b) internal pure returns (bytes1, bytes1) {
        return (_b[1], _b[0]);
    }
}

contract C {
    using L for bytes;

    function test(uint, bytes calldata _b, uint) external pure returns (bytes1, bytes1) {
        return _b.reverse();
    }
}

// ====
// compileViaYul: also
// ----
// test(uint256,bytes,uint256): 7, 0x60, 4, 2, "ab" -> "b", "a"
