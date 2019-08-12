contract Test {
    function bytesToBytes(bytes4 input) public returns (bytes2 ret) {
        return bytes2(input);
    }
}
// ====
// compileViaYul: also
// ----
// bytesToBytes(bytes4): "abcd" -> "ab"
