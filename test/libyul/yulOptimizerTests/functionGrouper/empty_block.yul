{ let a { } function f() -> x { let b := 4 {} for {} f() {} {} } }
// ----
// step: functionGrouper
//
// {
//     {
//         let a
//         { }
//     }
//     function f() -> x
//     {
//         let b := 4
//         { }
//         for { } f() { }
//         { }
//     }
// }
