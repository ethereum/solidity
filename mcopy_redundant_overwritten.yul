{
    calldatacopy(0, 0, 0x40)

    mcopy(0, 0x20, 0x20) // Redundant. Overwritten by MSTORE.
    mstore(0, 42)

    return(0, 0x40)
}
// ====
// EVMVersion: >=cancun
// ----
// step: fullSuite
//
// {
//     {
//         calldatacopy(0, 0, 0x40)
//         mcopy(0, 0x20, 0x20)
//         mstore(0, 42)
//         return(0, 0x40)
//     }
// }
