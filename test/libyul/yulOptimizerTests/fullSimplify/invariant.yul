{
    let a := calldataload(sub(7, 7))
    let b := sub(a, 0)
    // Below, `b` is not eliminated, because
    // we run CSE and then Simplify.
    // Elimination of `b` would require another
    // run of CSE afterwards.
    mstore(b, eq(calldataload(0), a))
}
// ----
// fullSimplify
// {
//     let a := calldataload(0)
//     let _4 := 0
//     let b := a
//     mstore(b, eq(calldataload(_4), a))
// }
