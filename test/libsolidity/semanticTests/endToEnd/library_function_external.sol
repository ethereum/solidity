library Lib {
    function m(bytes calldata b) external pure returns(byte) {
        return b[2];
    }
}
contract Test {
    function f(bytes memory b) public pure returns(byte) {
        return Lib.m(b);
    }
}

// ----

library Lib {
    function m(bytes calldata b) external pure returns(byte) {
        return b[2];
    }
}
contract Test {
    function f(bytes memory b) public pure returns(byte) {
        return Lib.m(b);
    }
}

// ----
// f(bytes): 0x20), 5), "abcde" -> "c"
// f(bytes):"32, 5, abcde" -> "c"
