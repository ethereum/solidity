{
  mstore(0x40, memoryguard(0x60))
  {
    let x, y
  }
  {
    let z, $w
  }
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0x80))
//     { let x, y }
//     {
//         let z_1, $w_2
//         mstore(0x60, $w_2)
//         let z := z_1
//     }
// }
