{
    mstore(calldataload(0), calldataload(10))
    if calldataload(1) {
        mstore(calldataload(0), 1)
    }
    let t := mload(0)
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
//         mstore(calldataload(0), calldataload(10))
//         let _5 := 1
//         if calldataload(_5) { mstore(calldataload(0), _5) }
//         let t := mload(0)
//         sstore(t, mload(calldataload(0)))
//     }
// }
