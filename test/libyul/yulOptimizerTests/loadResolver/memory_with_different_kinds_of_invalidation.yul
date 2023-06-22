{
    mstore(2, 9)
    sstore(0, mload(2))
    pop(call(0, 0, 0, 0, 0, 0, 0))
    sstore(0, mload(2))

    mstore(2, 10)
    mstore8(calldataload(0), 4)
    sstore(0, mload(2))

    mstore(2, 10)
    g()
    sstore(0, mload(2))

    function g() {}
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 9
//         let _2 := 2
//         mstore(_2, _1)
//         sstore(0, _1)
//         pop(call(0, 0, 0, 0, 0, 0, 0))
//         sstore(0, mload(_2))
//         let _8 := 10
//         mstore(_2, _8)
//         mstore8(calldataload(0), 4)
//         sstore(0, mload(_2))
//         mstore(_2, _8)
//         sstore(0, _8)
//     }
// }
