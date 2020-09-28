{
    let a := mload(0)
    let b := sub(a, a)
    sstore(0, b)
}
// ----
// step: expressionSimplifier
//
// { sstore(0, 0) }
