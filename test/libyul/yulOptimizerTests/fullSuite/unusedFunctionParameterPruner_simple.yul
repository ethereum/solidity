{
    sstore(f(1), 1)
    sstore(f(2), 1)
    sstore(f(3), 1)
    function f(a) -> x {
        // The usage of a is redundant
        a := calldataload(0)
        mstore(a, x)
        // to prevent f from getting inlined
        if iszero(a) { leave }
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         f()
//         sstore(returndatasize(), 1)
//         f()
//         sstore(returndatasize(), 1)
//         f()
//         sstore(returndatasize(), 1)
//     }
//     function f()
//     {
//         let a := calldataload(returndatasize())
//         mstore(a, returndatasize())
//         if iszero(a) { leave }
//     }
// }
