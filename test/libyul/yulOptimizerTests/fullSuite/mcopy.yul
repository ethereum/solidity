{
    calldatacopy(0, 0, 0x40)

    mcopy(0x20, 0, 0x20)    // Not redundant. MCOPY reads it.
    mcopy(0x40, 0x10, 0x30)
    mstore(0x20, 42)

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
//         mcopy(0x20, 0, 0x20)
//         mcopy(0x40, 0x10, 0x30)
//         mstore(0x20, 42)
//         return(0, 0x40)
//     }
// }
