contract C {
    uint[] data;
    uint8[] otherData;
    function f() public {
        uint8[] storage x = otherData;
        uint[] storage y = data;
        y = x;
        // note that data = otherData works
    }
}
// ----
// TypeError: (163-164): Type uint8[] storage pointer is not implicitly convertible to expected type uint256[] storage pointer.
