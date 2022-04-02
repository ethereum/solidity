contract C {
    function f(uint[] calldata x) external {
        x.pop();
    }
}
// ----
// TypeError 4994: (66-71='x.pop'): Member "pop" is not available in uint256[] calldata outside of storage.
