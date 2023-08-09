library L1 {
    event SameSignatureEvent(uint16);
}
library L2 {
    /// @notice This event is defined in library L2
    /// @dev This should not appear in Contract C devdoc
    event SameSignatureEvent(uint16);
}
library L3 {
    event SameSignatureEvent(uint16);
}
contract C {
    function f() public {
        emit L1.SameSignatureEvent(0);
        emit L2.SameSignatureEvent(1);
        emit L3.SameSignatureEvent(2);
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
