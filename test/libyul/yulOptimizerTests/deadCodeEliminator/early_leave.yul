{
    function f() -> x {
        for {
            let a := 20
        }
        lt(a, 40)
        {
            a := add(a, 2)
        }
        {
            a := a
            leave
            mstore(0, a)
            a := add(a, 10)
        }
        x := 9
    }
    pop(f())
}

// ====
// step: deadCodeEliminator
// ----
// {
//     function f() -> x
//     {
//         let a := 20
//         for { } lt(a, 40) { a := add(a, 2) }
//         {
//             a := a
//             leave
//         }
//         x := 9
//     }
//     pop(f())
// }
