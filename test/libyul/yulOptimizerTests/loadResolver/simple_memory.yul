{
    mstore(calldataload(0), calldataload(10))
    let t := mload(calldataload(10))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ====
// step: loadResolver
// ----
// {
//     let _2 := calldataload(10)
//     mstore(calldataload(0), _2)
//     sstore(mload(_2), _2)
// }
