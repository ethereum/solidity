contract C {
    function f(uint256[] memory x) public pure {
        x[1:2];
    }
}
// ----
// TypeError: (70-76): Index range access is only supported for dynamic calldata arrays.
