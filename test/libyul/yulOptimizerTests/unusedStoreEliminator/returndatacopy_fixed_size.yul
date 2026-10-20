// Unused `returndatacopy()` is not supposed be optimized out in this case.
// In fact, it's never optimized out because we removed this optimization from UnusedStoreEliminator.
{
  returndatacopy(0,0,32)
}
// ----
// step: unusedStoreEliminator
//
// { { returndatacopy(0, 0, 32) } }
