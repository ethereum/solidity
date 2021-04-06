{
    let a := calldataload(42)
    switch a
    case 0 {
        let b := calldataload(42)
        switch b
        case 0 { sstore(0, 1) }
        case 1 { sstore(0, 1) }
        case 2 { sstore(0, 1) }
        default { sstore(0, 1) }
    }
    default {
        let b := calldataload(42)
        switch b
        case 2 { sstore(0, 1) }
        case 1 { sstore(0, 1) }
        case 0 { sstore(0, 1) }
        default { sstore(0, 1) }
    }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     let a := calldataload(42)
//     let b := calldataload(42)
//     sstore(0, 1)
//     switch b
//     case 0 { }
//     case 1 { }
//     case 2 { }
//     default { }
//     switch a
//     case 0 { }
//     default { }
// }
