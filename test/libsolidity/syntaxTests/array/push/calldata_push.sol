contract C {
    function f(uint[] calldata x) external {
        x.push();
    }
}
// ----
// TypeError: (66-72): Member "push" is not available in uint256[] calldata outside of storage.
