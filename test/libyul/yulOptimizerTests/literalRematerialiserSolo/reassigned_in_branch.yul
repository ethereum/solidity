{
    let v := 2
    let a := v
    sstore(a, 1)
    if calldataload(0) { v := 3 }
    // `v` is no longer known, `a` still is.
    sstore(v, a)
}
// ----
// step: literalRematerialiserSolo
//
// {
//     let v := 2
//     let a := 2
//     sstore(2, 1)
//     if calldataload(0) { v := 3 }
//     sstore(v, 2)
// }
