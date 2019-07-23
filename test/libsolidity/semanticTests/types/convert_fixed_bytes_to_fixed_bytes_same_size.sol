contract Test {
    function bytesToBytes(bytes4 input) public returns (bytes4 ret) {
        return bytes4(input);
    }
}
// ====
// compileViaYul: also
// ----
// bytesToBytes(bytes4): "abcd" -> "abcd"
