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
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 9
//         let _2 := 2
//         mstore(_2, _1)
//         let _3 := _1
//         let _4 := 0
//         sstore(_4, _3)
//         pop(call(_4, _4, _4, _4, _4, _4, _4))
//         sstore(_4, mload(_2))
//         let _5 := 10
//         mstore(_2, _5)
//         mstore8(calldataload(_4), 4)
//         sstore(_4, mload(_2))
//         mstore(_2, _5)
//         sstore(_4, _5)
//     }
// }
