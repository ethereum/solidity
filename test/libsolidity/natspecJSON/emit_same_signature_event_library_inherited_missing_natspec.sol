contract D {
    event SameSignatureEvent(uint16);
}
library L {
    /// @notice This event is defined in library L
    /// @dev This should not appear in contract C devdoc
    event SameSignatureEvent(uint16);
}
contract C is D {
    function f() public {
        emit L.SameSignatureEvent(0);
        emit D.SameSignatureEvent(1);
    }
}

// ----
// ----
// :C devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :C userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
