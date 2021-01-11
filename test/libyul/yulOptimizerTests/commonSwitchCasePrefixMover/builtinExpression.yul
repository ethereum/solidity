{
  switch mload(0)
  case 0 {
    sstore(1, 0)
  }
  default {
    sstore(1, 0)
  }
  switch msize()
  case 0 {
    sstore(1, 0)
  }
  default {
    sstore(1, 0)
  }
  switch sload(0)
  case 0 {
    sstore(1, 0)
  }
  default {
    sstore(1, 0)
  }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     switch mload(0)
//     case 0 { sstore(1, 0) }
//     default { sstore(1, 0) }
//     switch msize()
//     case 0 { sstore(1, 0) }
//     default { sstore(1, 0) }
//     switch sload(0)
//     case 0 { sstore(1, 0) }
//     default { sstore(1, 0) }
// }
