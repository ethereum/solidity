{ let a:u256 { } function f() -> x:bool { let b:u256 := 4:u256 {} for {} f() {} {} } }
// ====
// step: functionGrouper
// yul: true
// ----
// {
//     {
//         let a:u256
//         {
//         }
//     }
//     function f() -> x:bool
//     {
//         let b:u256 := 4:u256
//         {
//         }
//         for {
//         }
//         f()
//         {
//         }
//         {
//         }
//     }
// }
