contract C {
    uint256[] x;
    function f() public view {
        x[1:2];
    }
}
// ----
// TypeError 1227: (69-75='x[1:2]'): Index range access is only supported for dynamic calldata arrays.
