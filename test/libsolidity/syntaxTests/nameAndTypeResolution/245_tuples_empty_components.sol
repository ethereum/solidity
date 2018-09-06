contract C {
    function f() public {
        (1,,2);
    }
}
// ----
// TypeError: (47-53): Tuple component cannot be empty.
