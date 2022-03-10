{
  mstore(0, 1)
  let x := call(
      0,
      0,
      0,
      0,
      0,
      0,
      0x32 // length is only a max length, so there is no guarantee that the mstore above is overwritten.
    )
  sstore(0, mload(0))
}

// ----
// step: unusedStoreEliminator
//
// {
//     {
//         mstore(0, 1)
//         let x := call(0, 0, 0, 0, 0, 0, 0x32)
//         sstore(0, mload(0))
//     }
// }
