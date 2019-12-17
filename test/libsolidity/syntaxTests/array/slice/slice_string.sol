contract C {
    function f() public pure {
        ""[1:];
    }
}
// ----
// TypeError: (52-58): Index range access is only possible for arrays and array slices.
