{
    let a := add(0, mload(0))
}
// ====
// step: expressionSimplifier
// ----
// { let a := mload(0) }
