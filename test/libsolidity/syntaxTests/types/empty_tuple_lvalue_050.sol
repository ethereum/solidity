pragma experimental "v0.5.0";
contract C {
    function f() private pure {}
    function a() public {
        uint x;
        uint y;
        (x, y) = (f(), f());
    }
}
// ----
// TypeError: (152-155): Tuple component cannot be empty.
