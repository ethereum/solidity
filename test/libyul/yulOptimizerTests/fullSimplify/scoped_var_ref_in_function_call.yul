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
//         let x
//         let _1 := calldataload(x)
//         let _2 := 1
//         x := add(_2, _1)
//         pop(call(2, 0, _2, addmod(_2, _1, 8), _2, _2, _2))
//     }
// }
