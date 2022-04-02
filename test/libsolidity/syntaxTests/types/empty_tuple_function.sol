contract C {
    function f() private pure {}
    function a() public pure {
        bool x = true;
        bool y = true;
        (x) ? (f(), y = false) : (f(), y = false);
    }
}
// ----
// TypeError 6473: (138-141='f()'): Tuple component cannot be empty.
// TypeError 6473: (157-160='f()'): Tuple component cannot be empty.
