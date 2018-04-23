pragma experimental "v0.5.0";
contract C {
    event SomeEvent();
    function a() public {
        (SomeEvent(), 7);
    }
}
// ----
// TypeError: (101-112): Event invocations have to be prefixed by "emit".
// TypeError: (101-112): Tuple component cannot be empty.
