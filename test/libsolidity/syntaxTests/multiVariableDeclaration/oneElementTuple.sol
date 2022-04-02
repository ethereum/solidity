contract C {
    function f() public {
        (uint a,) = (1,);
        a;
    }
}
// ----
// TypeError 8381: (59-63='(1,)'): Tuple component cannot be empty.
