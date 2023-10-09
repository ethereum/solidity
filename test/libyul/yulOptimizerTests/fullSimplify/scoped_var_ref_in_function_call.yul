{
    function f(x) {}
    let x
    {
        x := add(1, calldataload(0))
    }
    f(call(2, 0, 1, mod(x, 8), 1, 1, 1))
}
// ----
// step: fullSimplify
//
// {
//     {
//         let x_1
//         {
//             let _2 := calldataload(x_1)
//             let _3 := 1
//             x_1 := add(_3, _2)
//         }
//         let _4 := 1
//         pop(call(2, 0, _4, addmod(_3, _2, 8), _4, _4, _4))
//     }
// }
