contract C {
    function f() public {
        [msg];
    }
}
// ----
// TypeError 9656: (47-52='[msg]'): Unable to deduce nameable type for array elements. Try adding explicit type conversion for the first element.
