{
    let a:u256
    function f() {}
}
// ====
// step: mainFunction
// dialect: yul
// ----
// {
//     function main()
//     { let a:u256 }
//     function f()
//     { }
// }
