contract C {
    function f() public pure {
        string memory y;
        y[1:2];
    }
}
// ----
// TypeError: (77-83): Index range access is only supported for dynamic calldata arrays.
