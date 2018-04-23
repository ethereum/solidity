pragma solidity ^0.4.3;
contract C {
    event SomeEvent();
    function a() public {
        (SomeEvent(), 7);
    }
}
// ----
// Warning: (95-106): Invoking events without "emit" prefix is deprecated.
// Warning: (95-106): Tuple component cannot be empty.
