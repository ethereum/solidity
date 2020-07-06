{
    let x, y, z := foo(sload(0),sload(32))
    sstore(0, x)
    sstore(0, y)
    sstore(0, z)
    x, y, z := foo(sload(32), sload(8))

    // out3 is unassigned.
    function foo(a, b) -> out1, out2, out3
    {
        out1 := mload(32)
        out1 := sload(out1)
        out2 := add(out1, 1)
        extcodecopy(out1, out1, 1, b)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := sload(32)
//         let x, y, z := foo_24_75(sload(0), _1)
//         sstore(0, x)
//         sstore(0, y)
//         sstore(0, z)
//         let x_1, y_1, z_1 := foo_24_75(_1, sload(8))
//     }
//     function foo_24_75(a, b) -> out1, out2, out3
//     {
//         let out1_1 := sload(mload(32))
//         extcodecopy(out1_1, out1_1, 1, b)
//         out1 := out1_1
//         out2 := add(out1_1, 1)
//     }
// }
