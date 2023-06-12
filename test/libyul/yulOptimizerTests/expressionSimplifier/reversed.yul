{
    let a := add(0, mload(0))
    sstore(0, a)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// { { sstore(0, mload(0)) } }
