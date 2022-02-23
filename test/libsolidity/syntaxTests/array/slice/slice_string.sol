contract C {
    function f() public pure {
        ""[1:];
    }
}
// ----
// TypeError 4781: (52-58): Index range access is only possible for arrays and array slices.
