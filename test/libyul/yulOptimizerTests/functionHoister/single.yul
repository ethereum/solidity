{
    let a:u256
    function f() {}
}
// ====
// dialect: evmTyped
// ----
// step: functionHoister
//
// {
//     let a
//     function f()
//     { }
// }
