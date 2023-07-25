{
  let s := returndatasize()
  returndatacopy(0,0,s)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         returndatacopy(0, 0, returndatasize())
//     }
// }
