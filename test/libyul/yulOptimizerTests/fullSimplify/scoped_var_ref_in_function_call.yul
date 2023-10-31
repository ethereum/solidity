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
//         let _2 := calldataload(x_1)
//         let _3 := 1
//         x_1 := add(_3, _2)
//         pop(call(2, 0, _3, addmod(_3, _2, 8), _3, _3, _3))
//     }
// }
