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
// step: loadResolver
// ----
// {
//     let _1 := 9
//     let _2 := 2
//     mstore(_2, _1)
//     let _4 := _1
//     let _5 := 0
//     sstore(_5, _4)
//     pop(call(_5, _5, _5, _5, _5, _5, _5))
//     sstore(_5, mload(_2))
//     let _17 := 10
//     mstore(_2, _17)
//     mstore8(calldataload(_5), 4)
//     sstore(_5, mload(_2))
//     mstore(_2, _17)
//     g()
//     sstore(_5, mload(_2))
//     function g()
//     { }
// }
