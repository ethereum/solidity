pragma abicoder v2;

contract C {
    function g(bytes[2] memory m) internal returns (bytes memory) {
        return m[0];
    }
    function f(bytes[2] calldata c) external returns (bytes memory) {
        return g(c);
    }
}
// ----
// f(bytes[2]): 0x20, 0x40, 0x40, 2, "ab" -> 0x20, 2, "ab"
