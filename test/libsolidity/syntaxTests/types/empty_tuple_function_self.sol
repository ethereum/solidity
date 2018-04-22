pragma solidity ^0.4.3;
contract C {
    function a() public {
        (a(), 7);
    }
}
// ----
// TypeError: (72-75): Type of tuple component cannot be null.
