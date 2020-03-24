{
    let a:u256
    function f() {}
}
// ====
// dialect: yul
// ----
// step: functionHoister
//
// {
//     let a
//     function f()
//     { }
// }
