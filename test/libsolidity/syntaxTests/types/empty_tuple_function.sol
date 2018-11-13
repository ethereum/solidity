contract C {
    function f() private pure {}
    function a() public pure {
        bool x = true;
        bool y = true;
        (x) ? (f(), y = false) : (f(), y = false);
    }
}
// ----
// TypeError: (138-141): Tuple component cannot be empty.
// TypeError: (157-160): Tuple component cannot be empty.
