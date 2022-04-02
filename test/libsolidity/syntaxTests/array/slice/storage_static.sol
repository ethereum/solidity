contract C {
    uint256[42] x;
    function f() public view {
        x[1:2];
    }
}
// ----
// TypeError 1227: (71-77='x[1:2]'): Index range access is only supported for dynamic calldata arrays.
