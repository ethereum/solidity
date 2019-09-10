{
    let a:u256
    function f() {}
}
// ====
// step: mainFunction
// yul: true
// ----
// {
//     function main()
//     { let a:u256 }
//     function f()
//     { }
// }
