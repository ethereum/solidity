{
    let x := calldataload(0)
    switch x
    case 0 {
        x := calldataload(1)
    }
    default {
        x := calldataload(1)
    }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     let x := calldataload(0)
//     switch x
//     case 0 { x := calldataload(1) }
//     default { x := calldataload(1) }
// }
