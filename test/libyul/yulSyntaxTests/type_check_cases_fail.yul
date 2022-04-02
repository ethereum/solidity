{
    switch 7:i32
    case 0:i64 {}
    case 2:i64 {}
}
// ====
// dialect: ewasm
// ----
// TypeError 3781: (28-33='0:i64'): Expected a value of type "i32" but got "i64".
// TypeError 3781: (46-51='2:i64'): Expected a value of type "i32" but got "i64".
