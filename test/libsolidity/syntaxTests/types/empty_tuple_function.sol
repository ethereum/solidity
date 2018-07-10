pragma solidity ^0.4.3;
contract C {
    function f() private pure {}
    function a() public pure {
        bool x = true;
        bool y = true;
        (x) ? (f(), y = false) : (f(), y = false);
    }
}
// ----
// TypeError: (162-165): Tuple component cannot be empty.
// TypeError: (181-184): Tuple component cannot be empty.
