{
    mstore(calldataload(0), calldataload(10))
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
//         let _1 := calldataload(10)
//         let _2 := calldataload(0)
//         mstore(_2, _1)
//         let _3 := 1
//         if calldataload(_3) { mstore(add(_2, 0x20), _3) }
//         sstore(mload(add(_2, 0x20)), _1)
//     }
// }
