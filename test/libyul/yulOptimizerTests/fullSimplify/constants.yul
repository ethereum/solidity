{
    let a := add(1, mul(3, 4))
    mstore(0, a)
}
// ====
// step: fullSimplify
// ----
// { mstore(0, 13) }
