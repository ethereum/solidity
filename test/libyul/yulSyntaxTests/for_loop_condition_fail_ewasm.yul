{
    let x
    for {} x {} {}
}
// ====
// dialect: ewasm
// ----
// TypeError 1733: (23-24): Expected a value of boolean type "i32" but got "i64"
