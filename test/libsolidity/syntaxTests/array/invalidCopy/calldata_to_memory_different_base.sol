pragma abicoder v2;

contract C {
    function f(bytes8[] calldata c) external returns (uint256, bytes10, bytes10, bytes10) {
        bytes10[] memory m = c;
        return (m.length, m[0], m[1], m[2]);
    }
}
// ----
// TypeError 9574: (134-156='bytes10[] memory m = c'): Type bytes8[] calldata is not implicitly convertible to expected type bytes10[] memory.
