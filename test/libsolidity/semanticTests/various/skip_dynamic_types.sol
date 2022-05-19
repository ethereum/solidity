// The EVM cannot provide access to dynamically-sized return values, so we have to skip them.
contract C {
    function f() public returns (uint256, uint256[] memory, uint256) {
        return (7, new uint256[](2), 8);
    }

    function g() public returns (uint256, uint256) {
        // Previous implementation "moved" b to the second place and did not skip.
        (uint256 a, , uint256 b) = this.f();
        return (a, b);
    }
}
// ----
// g() -> 7, 8
