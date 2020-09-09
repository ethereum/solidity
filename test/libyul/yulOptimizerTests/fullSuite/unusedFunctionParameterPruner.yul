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
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := sload(32)
//         let out1 := sload(mload(32))
//         let out2 := add(out1, 1)
//         extcodecopy(out1, out2, 1, _1)
//         sstore(0, out1)
//         sstore(0, out2)
//         let _2 := sload(8)
//         let out1_1 := sload(mload(32))
//         extcodecopy(out1_1, add(out1_1, 1), 1, _2)
//     }
// }
