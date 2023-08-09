contract D {
    /// @notice This event is defined in contract D
    /// @dev This should appear in Contract C dev doc
    event SameSignatureEvent(uint16);
}
library L {
    /// @notice This event is defined in Library L
    /// @dev This should not appear in Contract C
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
//     "events":
//     {
//         "SameSignatureEvent(uint16)":
//         {
//             "details": "This should appear in Contract C dev doc"
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :C userdoc
// {
//     "events":
//     {
//         "SameSignatureEvent(uint16)":
//         {
//             "notice": "This event is defined in contract D"
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
