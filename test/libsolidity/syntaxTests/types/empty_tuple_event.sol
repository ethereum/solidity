pragma solidity ^0.4.3;
contract C {
    event SomeEvent();
    function a() public {
        (emit SomeEvent(), 7);
    }
}
// ----
// TypeError: (95-106): Event invocations have to be prefixed by "emit".
// Warning: (95-106): Tuple component cannot be empty.
