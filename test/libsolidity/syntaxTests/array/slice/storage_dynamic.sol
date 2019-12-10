contract C {
    uint256[] x;
    function f() public view {
        x[1:2];
    }
}
// ----
// TypeError: (69-75): Index range access is only supported for dynamic calldata arrays.
