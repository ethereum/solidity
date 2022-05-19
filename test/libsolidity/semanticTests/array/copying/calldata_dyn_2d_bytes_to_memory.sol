pragma abicoder v2;

contract C {
    function f(bytes[] calldata c) external returns (bytes[] memory) {
        return c;
    }
}
// ----
// f(bytes[]): 0x20, 2, 0x60, 0x60, 0x20, 2, "ab" -> 0x20, 2, 0x40, 0x80, 2, "ab", 2, "ab"
