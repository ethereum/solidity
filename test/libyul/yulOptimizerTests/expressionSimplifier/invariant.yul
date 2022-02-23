{
    let a := mload(sub(7, 7))
    let b := sub(a, 0)
    sstore(0, b)
}
// ----
// step: expressionSimplifier
//
// { { sstore(0, mload(0)) } }
