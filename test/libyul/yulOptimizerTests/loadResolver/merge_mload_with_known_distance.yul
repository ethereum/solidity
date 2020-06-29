{
    let x := mload(calldataload(0))
    if calldataload(1) {
        mstore(add(calldataload(0), 0x20), 1)
    }
    let t := mload(add(calldataload(0), 0x20))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ----
// step: loadResolver
//
// {
//     let _2 := calldataload(0)
//     let x := mload(_2)
//     let _3 := 1
//     if calldataload(_3) { mstore(add(_2, 0x20), _3) }
//     sstore(mload(add(_2, 0x20)), x)
// }
