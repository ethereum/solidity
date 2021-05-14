{
    switch calldataload(42)
    case 0 {
        sstore(0, 1)
    }
    case 1 {
        sstore(0, 1)
    }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     switch calldataload(42)
//     case 0 { sstore(0, 1) }
//     case 1 { sstore(0, 1) }
// }
