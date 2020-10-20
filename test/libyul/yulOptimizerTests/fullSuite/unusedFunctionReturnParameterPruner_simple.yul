{
    let c, d := f(calldataload(0), calldataload(1))
    sstore(c, 1)
    function f(x, y) -> a, b
    {
        // to prevent the function getting inlined
        if iszero(calldataload(0)) {leave}
        a := sload(x)
        b := sload(y)
    }
}
// ----
// step: fullSuite
//
// {
//     { sstore(f(calldataload(0)), 1) }
//     function f(x) -> a
//     {
//         if iszero(calldataload(a)) { leave }
//         a := sload(x)
//     }
// }
