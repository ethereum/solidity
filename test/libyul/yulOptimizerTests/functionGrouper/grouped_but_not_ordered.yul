{
    function f() -> y { y := 8 }
    {
        let x := 2
    }
}
// ====
// step: functionGrouper
// ----
// {
//     {
//         {
//             let x := 2
//         }
//     }
//     function f() -> y
//     {
//         y := 8
//     }
// }
