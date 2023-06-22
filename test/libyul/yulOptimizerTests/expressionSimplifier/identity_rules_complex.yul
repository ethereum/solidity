{
    let a := sub(calldataload(0), calldataload(0))
    sstore(0, a)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// { { sstore(0, 0) } }
