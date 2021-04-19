{
    mstore(0, 30)
    if calldataload(0) {
        mstore(0, 20)
    }
    let val := keccak256(0, 32)
    sstore(0, val)
}
// ----
// step: loadResolver
//
// {
//     let _1 := 30
//     let _2 := 0
//     mstore(_2, _1)
//     if calldataload(_2) { mstore(_2, 20) }
//     sstore(_2, keccak256(_2, 32))
// }
