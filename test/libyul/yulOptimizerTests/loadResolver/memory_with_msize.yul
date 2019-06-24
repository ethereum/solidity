{
    // No mload removal because of msize
    mstore(calldataload(0), msize())
    let t := mload(calldataload(10))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ====
// step: loadResolver
// ----
// {
//     let _1 := msize()
//     let _3 := calldataload(0)
//     mstore(_3, _1)
//     let t := mload(calldataload(10))
//     sstore(t, mload(_3))
// }
