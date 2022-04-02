{
    let x
    for {} x {} {}
}
// ====
// dialect: evmTyped
// ----
// TypeError 1733: (23-24='x'): Expected a value of boolean type "bool" but got "u256"
