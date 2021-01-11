{
  let a := calldataload(42)
  switch a
  case 0 {
    let b := calldataload(23)
    switch b
    case 0 { sstore(0, 1) }
    default { sstore(0, 1) }
  }
  default { sstore(1, 2) }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     let a := calldataload(42)
//     switch a
//     case 0 {
//         let b := calldataload(23)
//         sstore(0, 1)
//         switch b
//         case 0 { }
//         default { }
//     }
//     default { sstore(1, 2) }
// }
