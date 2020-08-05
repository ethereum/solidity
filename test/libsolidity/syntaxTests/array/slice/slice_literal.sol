contract C {
    function f() public pure {
        1[1:];
    }
}
// ----
// TypeError 4781: (52-57): Index range access is only possible for arrays and array slices.
