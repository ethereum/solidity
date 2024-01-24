{
    calldatacopy(0, 0, 0x40)

    mcopy(0, 0x20, 0) // Redundant. Does not copy anything.

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
//         mcopy(0, 0, 0)
//         return(0, 0x40)
//     }
// }
