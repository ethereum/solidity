{
    switch 7:i32
    case 0:i64 {}
    case 2:i64 {}
}
// ====
// dialect: ewasm
// ----
// TypeError: (28-33): Expected a value of type "i32" but got "i64"
// TypeError: (46-51): Expected a value of type "i32" but got "i64"
