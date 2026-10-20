// `s` captures returndatasize() after the first call.
// The second call resets the return data buffer, making `s` stale.
// `returndatacopy` using the stale `s` must NOT be eliminated.
{
    pop(staticcall(gas(), 0, 0, 0, 0, 0))
    let s := returndatasize()
    pop(staticcall(gas(), 0, 0, 0, 0, 0))
    returndatacopy(0, 0, s)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         pop(staticcall(gas(), 0, 0, 0, 0, 0))
//         let s := returndatasize()
//         pop(staticcall(gas(), 0, 0, 0, 0, 0))
//         returndatacopy(0, 0, s)
//     }
// }
