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
//     {
//         let _1 := 1337
//         let _2 := 0
//         mstore(_2, _1)
//         funcWithLoop(42)
//         sstore(_2, mload(_2))
//     }
//     function userNot(x) -> y
//     { y := iszero(x) }
//     function funcWithLoop(x_1)
//     {
//         let _7 := 0
//         mstore(_7, _7)
//         for { } userNot(x_1) { }
//         { }
//     }
// }
