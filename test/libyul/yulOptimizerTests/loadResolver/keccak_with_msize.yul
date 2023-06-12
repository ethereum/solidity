{
    sstore(0, msize())
    mstore(0, 10)
    // Keccak-256 should not be evaluated because of the msize.
    // Even though, though the code is likely equivalent,
    // we skip steps if `msize` is present
    let val := keccak256(0, 32)
    sstore(1, val)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         sstore(0, msize())
//         mstore(0, 10)
//         sstore(1, keccak256(0, 32))
//     }
// }
