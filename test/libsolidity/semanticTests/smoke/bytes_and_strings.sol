contract C {
    function e(bytes memory b) public pure returns (bytes memory) {
        return b;
    }
    function f() public pure returns (string memory, string memory) {
        return ("any", "any");
    }
    function g() public pure returns (string memory, uint, string memory) {
        return ("any", 42, "any");
    }
    function h() public pure returns (string memory) {
        return "any";
    }
}
// ----
// e(bytes): 32, 3, hex"AB33BB" -> 32, 3, left(0xAB33BB)
// e(bytes): 32, 32, 0x20 -> 32, 32, 0x20
// e(bytes): 32, 3, hex"AB33FF" -> 32, 3, hex"ab33ff0000000000000000000000000000000000000000000000000000000000"
// f() -> 0x40, 0x80, 3, "any", 3, "any"
// g() -> 0x60, 0x2a, 0xa0, 3, "any", 3, "any"
// h() -> 0x20, 3, "any"

