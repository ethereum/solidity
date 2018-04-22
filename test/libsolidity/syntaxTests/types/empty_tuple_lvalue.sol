pragma solidity ^0.4.3;
contract C {
    function f() public pure {}
    function a() public {
        uint x;
        uint y;
        (x, y) = (f(), f());
    }
}
// ----
// TypeError: (145-148): Type of tuple component cannot be null.
