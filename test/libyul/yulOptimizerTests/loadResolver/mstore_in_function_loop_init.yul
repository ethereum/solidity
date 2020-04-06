{
    function userNot(x) -> y {
        y := iszero(x)
    }

    function funcWithLoop(x) {
        for { mstore(0, 0) } userNot(x) {} {}
    }

    mstore(0, 1337)
    funcWithLoop(42)
    sstore(0, mload(0))
}
// ----
// step: loadResolver
//
// {
//     function userNot(x) -> y
//     { y := iszero(x) }
//     function funcWithLoop(x_1)
//     {
//         let _1 := 0
//         mstore(_1, _1)
//         for { } userNot(x_1) { }
//         { }
//     }
//     let _3 := 1337
//     let _4 := 0
//     mstore(_4, _3)
//     funcWithLoop(42)
//     sstore(_4, mload(_4))
// }
