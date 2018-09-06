contract C {
    function f() public {
        (uint a,) = (1,);
        a;
    }
}
// ----
// TypeError: (59-63): Tuple component cannot be empty.
