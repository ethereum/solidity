{
    function f(a1, a2, a3, a4, a5, a6, a7, a8) -> a9, a10, a11, a12, a13, a14, a15, a16 {
    }
    function g() -> a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16 {}
    function h() -> a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16 {}
    pop(memoryguard(0))
    let b9, b10, b11, b12, b13, b14, b15, b16 := f(1, 2, 3, 4, 5, 6, 7, 8)
    let c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16 := g()
    let d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16 := h()
}
// ----
// step: stackLimitEvader
//
// {
//     function f(a1, a2, a3, a4, a5, a6, a7, a8) -> a9, a10, a11, a12, a13, a14, a15, a16
//     { }
//     function g() -> a1_1, a2_2, a3_3, a4_4, a5_5, a6_6, a7_7, a8_8, a9_9, a10_10, a11_11, a12_12, a13_13, a14_14, a15_15, a16_16
//     { }
//     function h() -> a1_17, a2_18, a3_19, a4_20, a5_21, a6_22, a7_23, a8_24, a9_25, a10_26, a11_27, a12_28, a13_29, a14_30, a15_31, a16_32
//     { }
//     pop(memoryguard(0x00))
//     let b9, b10, b11, b12, b13, b14, b15, b16 := f(1, 2, 3, 4, 5, 6, 7, 8)
//     let c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16 := g()
//     let d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16 := h()
// }
