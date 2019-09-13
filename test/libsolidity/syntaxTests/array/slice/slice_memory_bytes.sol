contract C {
    function f() public pure {
        bytes memory y;
        y[1:2];
    }
}
// ----
// TypeError: (76-82): Index range access is only supported for dynamic calldata arrays.
