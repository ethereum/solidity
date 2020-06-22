contract C {
    function f() public {
        [type(C)];
    }
}
// ----
// TypeError 9656: (47-56): Unable to deduce nameable type for array elements. Try adding explicit type conversion for the first element.
