{
    {
        let x := 2
    }
    function f() -> y { y := 8 }
}
// ----
// functionGrouper
// {
//     {
//         let x := 2
//     }
//     function f() -> y
//     {
//         y := 8
//     }
// }
