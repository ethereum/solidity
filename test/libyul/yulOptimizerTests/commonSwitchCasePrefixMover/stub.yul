{
  let x := calldataload(0)
  switch x
  case 0 {
    sstore(0, 1)
    sstore(1, 2)
    sstore(2, 3)
  }
  default {
    sstore(0, 1)
    sstore(1, 2)
    sstore(2, 4)
  }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     let x := calldataload(0)
//     sstore(0, 1)
//     sstore(1, 2)
//     switch x
//     case 0 { sstore(2, 3) }
//     default { sstore(2, 4) }
// }
