contract C {
    function f(uint[] calldata x) external {
        x.push();
    }
}
// ----
// TypeError 4994: (66-72): Member "push" is not available in uint256[] calldata outside of storage.
