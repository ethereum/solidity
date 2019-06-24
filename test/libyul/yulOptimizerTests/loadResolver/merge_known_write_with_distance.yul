{
    mstore(calldataload(0), calldataload(10))
    if calldataload(1) {
        mstore(add(calldataload(0), 0x20), 1)
    }
    let t := mload(add(calldataload(0), 0x20))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ====
// step: loadResolver
// ----
// {
//     let _2 := calldataload(10)
//     let _4 := calldataload(0)
//     mstore(_4, _2)
//     let _5 := 1
//     if calldataload(_5) { mstore(add(_4, 0x20), _5) }
//     sstore(mload(add(_4, 0x20)), _2)
// }
