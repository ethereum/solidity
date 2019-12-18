{
    let a:u256
    function f() {}
}
// ====
// step: functionHoister
// dialect: yul
// ----
// {
//     let a:u256
//     function f()
//     { }
// }
