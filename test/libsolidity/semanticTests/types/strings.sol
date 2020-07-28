contract test {
    function fixedBytesHex() public returns(bytes32 ret) {
        return hex"aabb00ff";
    }
    function fixedBytes() public returns(bytes32 ret) {
        return "abc\x00\xff__";
    }
    function pipeThrough(bytes2 small, bool one) public returns(bytes16 large, bool oneRet) {
        oneRet = one;
        large = small;
    }
}

// ====
// compileViaYul: also
// ----
// fixedBytesHex() -> "\xaa\xbb\0\xff"
// fixedBytes() -> "abc\0\xff__"
// pipeThrough(bytes2, bool): "\0\x02", true -> "\0\x2", true
