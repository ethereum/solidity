{
    mstore(0x40, 7)
    let d := staticcall(10000, 10, 0, 200, 0, 0)
    sstore(0, mload(0x40))
    mstore(0x80, 7)
    calldatacopy(0, 0, 0)
    sstore(0, mload(0x80))
}
// ====
// EVMVersion: >=byzantium
// ----
// step: loadResolver
//
// {
//     let _1 := 7
//     mstore(0x40, _1)
//     let _3 := 0
//     pop(staticcall(10000, 10, _3, 200, _3, _3))
//     sstore(_3, _1)
//     mstore(0x80, _1)
//     calldatacopy(_3, _3, _3)
//     sstore(_3, _1)
// }
