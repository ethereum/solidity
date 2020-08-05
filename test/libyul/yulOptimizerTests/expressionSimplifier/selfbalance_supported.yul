{
    let ret := balance(address())
}
// ====
// EVMVersion: >=istanbul
// ----
// step: expressionSimplifier
//
// { let ret := selfbalance() }
