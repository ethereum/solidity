{
    mstore(calldataload(0), calldataload(10))
    if calldataload(1) {
        mstore(0, 1)
    }
    let t := mload(0)
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ====
// step: loadResolver
// ----
// {
//     let _2 := calldataload(10)
//     let _3 := 0
//     let _4 := calldataload(_3)
//     mstore(_4, _2)
//     let _5 := 1
//     if calldataload(_5) { mstore(_3, _5) }
//     let t := mload(_3)
//     sstore(t, mload(_4))
// }
