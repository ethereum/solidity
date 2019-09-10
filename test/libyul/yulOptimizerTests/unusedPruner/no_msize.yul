{
    let a := 1
    let b := mload(10)
    sstore(0, 5)
}
// ====
// step: unusedPruner
// ----
// { sstore(0, 5) }
