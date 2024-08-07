{ let a:u256 { } function f() -> x:bool { let b:u256 := 4:u256 {} for {} f() {} {} } }
// ====
// dialect: evmTyped
// ----
// step: functionGrouper
//
// {
//     {
//         let a
//         { }
//     }
//     function f() -> x:bool
//     {
//         let b := 4
//         { }
//         for { } f() { }
//         { }
//     }
// }
