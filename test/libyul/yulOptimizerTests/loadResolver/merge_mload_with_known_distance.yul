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
//     {
//         let _1 := calldataload(0)
//         let x := mload(_1)
//         let _2 := 1
//         if calldataload(_2) { mstore(add(_1, 0x20), _2) }
//         sstore(mload(add(_1, 0x20)), x)
//     }
// }
