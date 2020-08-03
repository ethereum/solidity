{
    sstore(0, balance(address()))
}
// ====
// EVMVersion: >=istanbul
// ----
// step: expressionSimplifier
//
// { sstore(0, selfbalance()) }
