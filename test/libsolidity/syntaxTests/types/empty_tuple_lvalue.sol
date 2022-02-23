contract C {
    function f() private pure {}
    function a() public {
        uint x;
        uint y;
        (x, y) = (f(), f());
    }
}
// ----
// TypeError 6473: (122-125): Tuple component cannot be empty.
// TypeError 6473: (127-130): Tuple component cannot be empty.
// TypeError 7407: (121-131): Type tuple(tuple(),tuple()) is not implicitly convertible to expected type tuple(uint256,uint256).
