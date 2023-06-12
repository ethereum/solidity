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
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         mstore(0, 1337)
//         funcWithLoop(42)
//         sstore(0, mload(0))
//     }
//     function userNot(x) -> y
//     { y := iszero(x) }
//     function funcWithLoop(x_1)
//     {
//         mstore(0, 0)
//         for { } userNot(x_1) { }
//         { }
//     }
// }
