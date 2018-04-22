pragma solidity ^0.4.3;
contract C {
    function f() {}
    function a() public {
        bool x = true;
        bool y = true;
        (x) ? (f(), y = false) : (f(), y = false);
    }
}
// ----
// TypeError: (144-147): Type of tuple component cannot be null.
