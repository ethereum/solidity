{
    // cannot be removed because of verbatim
    let a := mload(10)
    // cannot be removed because of verbatim
    let b := keccak256(10, 32)
    // can be removed
    let c := add(a, b)
    verbatim_0i_0o("test")
}
// ----
// step: unusedPruner
//
// {
//     {
//         pop(mload(10))
//         pop(keccak256(10, 32))
//         verbatim_0i_0o("test")
//     }
// }
