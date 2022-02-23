contract C {
    function f() public pure {
        string memory y;
        y[1:2];
    }
}
// ----
// TypeError 1227: (77-83): Index range access is only supported for dynamic calldata arrays.
