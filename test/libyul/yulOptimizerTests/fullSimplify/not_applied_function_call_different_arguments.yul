{
    function f(a) -> b { }
    mstore(0, sub(f(0), f(1)))
}
// ----
// step: fullSimplify
//
// {
//     {
//         let _1 := f(1)
//         let _2 := 0
//         mstore(_2, sub(f(_2), _1))
//     }
//     function f(a) -> b
//     { }
// }
