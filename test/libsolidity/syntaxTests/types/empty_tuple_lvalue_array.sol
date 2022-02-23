contract C {
    function f() private pure {}
    function a() public {
        uint x;
        uint y;
        (x, y) = [f(), f()];
    }
}
// ----
// TypeError 5604: (122-125): Array component cannot be empty.
