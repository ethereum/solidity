// This test ensures that `returndatacopy` is NOT optimized away.
{
  returndatacopy(0,0,returndatasize())
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         returndatacopy(0, 0, returndatasize())
//     }
// }
