pragma solidity ^0.4.3;
contract C {
    event SomeEvent();
    function a() public {
        (SomeEvent(), 7);
    }
}
// ----
// Warning: (95-106): Invoking events without "emit" prefix is deprecated.
// TypeError: (95-106): Type of tuple component cannot be null.
