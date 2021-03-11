{
    for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
        g()
        mstore(i, 0)
    }

    for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
        f()
        sstore(0, call(0, 0, 0, 0, 0, 0, 0))
    }

    function f() {
        sstore(0, 0)
    }
    function g() {
        sstore(0, 0)
    }
}
// ----
// step: zeroByReturndatasizeReplacer
//
// {
//     for { let i := returndatasize() } lt(i, 4) { i := add(i, 1) }
//     {
//         g()
//         mstore(i, returndatasize())
//     }
//     for { let i_1 := 0 } lt(i_1, 4) { i_1 := add(i_1, 1) }
//     {
//         f()
//         sstore(0, call(0, 0, 0, 0, 0, 0, 0))
//     }
//     function f()
//     { sstore(0, 0) }
//     function g()
//     {
//         sstore(returndatasize(), returndatasize())
//     }
// }
