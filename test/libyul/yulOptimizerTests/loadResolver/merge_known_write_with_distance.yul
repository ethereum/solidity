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
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _2 := calldataload(10)
//         let _3 := calldataload(0)
//         mstore(_3, _2)
//         let _4 := 1
//         if calldataload(_4) { mstore(add(_3, 0x20), _4) }
//         sstore(mload(add(_3, 0x20)), _2)
//     }
// }
