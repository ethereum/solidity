{
    let a:u256
    function f() {}
}
// ====
// dialect: yul
// ----
// step: mainFunction
//
// {
//     function main()
//     { let a }
//     function f()
//     { }
// }
