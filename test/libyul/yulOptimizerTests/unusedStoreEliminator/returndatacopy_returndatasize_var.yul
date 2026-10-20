// This test ensures that `returndatacopy` is NOT optimized away.
{
  let s := returndatasize()
  returndatacopy(0,0,s)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         returndatacopy(0, 0, returndatasize())
//     }
// }
