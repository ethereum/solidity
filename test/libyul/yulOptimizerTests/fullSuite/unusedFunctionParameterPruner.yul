{
    let x, y := foo(sload(0),sload(32))
    sstore(0, x)
    sstore(0, y)
    x, y := foo(sload(32), sload(8))

    function foo(a, b) -> out1, out2
    {
        out1 := mload(32)
        out1 := sload(out1)
        out2 := add(out1, 1)
        extcodecopy(out1, out2, 1, b)
        // to prevent foo from getting inlined
        if iszero(out1) { leave }
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let out1, out2 := foo(sload(32))
//         sstore(0, out2)
//         let out1_1, out2_1 := foo(sload(8))
//     }
//     function foo(b) -> out1, out2
//     {
//         out1 := sload(mload(32))
//         out2 := add(out1, 1)
//         extcodecopy(out1, out2, 1, b)
//         if iszero(out1) { leave }
//     }
// }
