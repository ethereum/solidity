contract C {
    function f() public returns (string memory, uint256) {
        return ("abc", 8);
    }

    function g() public returns (string memory, string memory) {
        return (h(), "def");
    }

    function h() public returns (string memory) {
        return ("abc");
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x40, 0x8, 0x3, "abc"
// g() -> 0x40, 0x80, 0x3, "abc", 0x3, "def"
