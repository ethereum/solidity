{
    function g() -> r1, r2
    {
        r1 := calldataload(0)
        r2 := add(calldataload(0), 32)
    }
    let a, b := g() // Multi-variable declarations are never tracked
    let c := add(a, 32)
    // [c, c+32) may overlap [a, a+b) for some values of b (the read length), so this
    // could not be proven unused even if a and b were tracked and known to be related.
    mstore(c, 0xAA)
    return(a, b)
}
// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     function g() -> r1, r2
//     {
//         r1 := calldataload(0)
//         r2 := add(calldataload(0), 32)
//     }
//     let a, b := g()
//     let c := add(a, 32)
//     mstore(c, 0xAA)
//     return(a, b)
// }
