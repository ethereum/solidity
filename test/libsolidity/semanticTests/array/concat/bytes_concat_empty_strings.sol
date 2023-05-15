contract C {
    function f() public returns (bytes memory) {
        bytes memory b = "";
        return bytes.concat(
            bytes.concat(b),
            bytes.concat(b, b),
            bytes.concat("", b),
            bytes.concat(b, "")
        );
    }

    function g() public returns (bytes memory) {
        return bytes.concat("", "abc", hex"", "abc", unicode"");
    }

    function h() public returns (bytes memory) {
        bytes memory b = "";
        return bytes.concat(b, "abc", b, "abc", b);
    }
}
// ----
// f() -> 0x20, 0
// g() -> 0x20, 6, "abcabc"
// h() -> 0x20, 6, "abcabc"
