contract C {
    function f() public pure returns (bytes2) {
        return bytes2(bin"000100100011010001010110");
    }
}
// ----
// TypeError 9640: (76-113): Explicit type conversion not allowed from "literal_string hex"123456"" to "bytes2". Literal is larger than the type.
