contract C {
    function f() public {
        [msg];
    }
}
// ----
// TypeError: (47-52): Unable to deduce nameable type for array elements. Try adding explicit type conversion for the first element.
