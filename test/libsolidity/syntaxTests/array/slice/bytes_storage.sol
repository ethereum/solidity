contract C {
    bytes x;
    function f() public view {
        x[1:2];
    }
}
// ----
// TypeError: (65-71): Index range access is only supported for dynamic calldata arrays.
