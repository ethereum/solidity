{
    let a := calldataload(42)
    switch a
    case 0 {
        let b := calldataload(42)
        switch b
        case 0 { sstore(0, 1) }
        default { sstore(0, 1) }
    }
    default {
        let c := calldataload(42)
        sstore(0, 1)
        switch c
        case 0 {}
        default {}
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
//     default { }
//     switch a
//     case 0 { }
//     default { }
// }
