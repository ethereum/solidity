{
    function f(a, b) -> x {
        let t
        a := 2
        x := 2
        t := 2
        if b { leave }
        a := 8
        x := 8
        t := 8
    }
    function g(a, b) -> x {
        let t
        a := 2
        x := 2
        t := 2
        if b { }
        a := 8
        x := 8
        t := 8
    }
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     function f(a, b) -> x
//     {
//         let t
//         x := 2
//         if b { leave }
//         x := 8
//     }
//     function g(a_1, b_2) -> x_3
//     {
//         let t_4
//         if b_2 { }
//         x_3 := 8
//     }
// }
