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
// Warning: (146-149): Tuple component cannot be empty.
// Warning: (151-154): Tuple component cannot be empty.
// TypeError: (145-155): Type tuple()[2] memory is not implicitly convertible to expected type tuple(uint256,uint256).
