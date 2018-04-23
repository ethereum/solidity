pragma experimental "v0.5.0";
contract C {
    function f() private pure {}
    function a() public pure {
        bool x = true;
        bool y = true;
        (x) ? (f(), y = false) : (f(), y = false);
    }
}
// ----
// TypeError: (168-171): Tuple component cannot be empty.
