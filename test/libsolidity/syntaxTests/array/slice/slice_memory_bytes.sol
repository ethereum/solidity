contract C {
    function f() public pure {
        bytes memory y;
        y[1:2];
    }
}
// ----
// TypeError 1227: (76-82='y[1:2]'): Index range access is only supported for dynamic calldata arrays.
