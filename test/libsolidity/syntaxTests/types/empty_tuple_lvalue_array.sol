pragma solidity ^0.4.3;
contract C {
    function f() private pure {}
    function a() public {
        uint x;
        uint y;
        (x, y) = [f(), f()];
    }
}
// ----
// TypeError: (146-149): Array component cannot be empty.
