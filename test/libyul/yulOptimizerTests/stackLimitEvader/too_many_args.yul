{
    function f(a1, a2, a3, a4, a5, a6, a7, a8) -> a9, a10, a11, a12, a13, a14, a15, a16, a17 {
    }
    pop(memoryguard(0))
    let b9, b10, b11, b12, b13, b14, b15, b16, b17 := f(1, 2, 3, 4, 5, 6, 7, 8)
}
// ----
// step: stackLimitEvader
//
// {
//     function f(a1, a2, a3, a4, a5, a6, a7) -> a9, a10, a11, a12, a13, a14, a15, a16, a17
//     { }
//     pop(memoryguard(0x20))
//     let _2 := 8
//     let _3 := 7
//     let _4 := 6
//     let _5 := 5
//     let _6 := 4
//     let _7 := 3
//     let _8 := 2
//     let _9 := 1
//     mstore(0x00, _2)
//     let b9, b10, b11, b12, b13, b14, b15, b16, b17 := f(_9, _8, _7, _6, _5, _4, _3)
// }
