library L {
    /// @notice This event is defined in library L
    /// @dev This should not appear in contract C devdoc
    event SameSignatureEvent(uint16);
    /// @notice This event is defined in library L
    /// @dev This should appear in contract C devdoc
    event LibraryEvent(uint32);
}
contract C {
    event SameSignatureEvent(uint16);
    /// @notice This event is defined in contract C
    event ContractEvent(uint32);
    function f() public {
        emit L.SameSignatureEvent(0);
        emit SameSignatureEvent(1);
        emit L.LibraryEvent(2);
        emit ContractEvent(3);
    }
}

// ----
// ----
// :C devdoc
// {
//     "events":
//     {
//         "LibraryEvent(uint32)":
//         {
//             "details": "This should appear in contract C devdoc"
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
//         "ContractEvent(uint32)":
//         {
//             "notice": "This event is defined in contract C"
//         },
//         "LibraryEvent(uint32)":
//         {
//             "notice": "This event is defined in library L"
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
