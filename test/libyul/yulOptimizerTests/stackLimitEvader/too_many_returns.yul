{
    function g() -> a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17 {}
    pop(memoryguard(0))
    let b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17 := g()
}
// ----
// step: stackLimitEvader
//
// {
//     function g() -> a1, a2, a3, a4, a5, a6, a7, a9, a10, a11, a12, a13, a14, a15, a16, a17
//     { mstore(0x00, 0) }
//     pop(memoryguard(0x20))
//     let b1, b2, b3, b4, b5, b6, b7, b9, b10, b11, b12, b13, b14, b15, b16, b17 := g()
//     let b8 := mload(0x00)
// }
