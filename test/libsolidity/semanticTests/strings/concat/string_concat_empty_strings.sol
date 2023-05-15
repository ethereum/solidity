contract C {
    function f() public returns (string memory) {
        string memory b = "";
        return string.concat(
            string.concat(b),
            string.concat(b, b),
            string.concat("", b),
            string.concat(b, "")
        );
    }

    function g() public returns (string memory) {
        return string.concat("", "abc", hex"", "abc", unicode"");
    }

    function h() public returns (string memory) {
        string memory b = "";
        return string.concat(b, "abc", b, "abc", b);
    }
}
// ----
// f() -> 0x20, 0
// g() -> 0x20, 6, "abcabc"
// h() -> 0x20, 6, "abcabc"
