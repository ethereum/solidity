contract C {
    function f() public pure returns (bytes2) {
        return bytes2(hex"123456");
    }
}
// ----
// TypeError 9640: (76-95='bytes2(hex"123456")'): Explicit type conversion not allowed from "literal_string hex"123456"" to "bytes2". Literal is larger than the type.
