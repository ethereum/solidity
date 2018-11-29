contract C {
    function f() public {
        uint8[4][4] memory dyn = [[1, 2, 3, 4], [2, 3, 4, 5], [3, 4, 5, 6], [4, 5, 6, 7]];
    }
}
// ----
// Warning: (47-69): Unused local variable.
// Warning: (17-135): Function state mutability can be restricted to pure
