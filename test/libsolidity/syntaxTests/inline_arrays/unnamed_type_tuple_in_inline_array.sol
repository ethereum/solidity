contract C {
    function f() public {
        [(1, 2, 3), (4, 5, 6)];
    }
}
// ----
// TypeError 9656: (47-69): Unable to deduce nameable type for array elements. Try adding explicit type conversion for the first element.
