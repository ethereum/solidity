contract Test {
    string constant x = "abefghijklmnopqabcdefghijklmnopqabcdefghijklmnopqabca";
    function f() public {
        // Even if this is made possible in the future, we should not allow assignment
        // to elements of constant arrays.
        x[0] = "f";
    }
}
// ----
// TypeError: (261-265): Index access for string is not possible.
