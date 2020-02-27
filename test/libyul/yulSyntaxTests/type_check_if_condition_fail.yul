{
    let x:i64
    if x {}
}
// ====
// dialect: ewasm
// ----
// TypeError: (23-24): Expected a value of boolean type "i32" but got "i64"
