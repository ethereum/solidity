{
    {
        mstore(0x40, memoryguard(128))
	let a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15 := g(16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30)
        sstore(0, 1)
	a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15 := g(16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30)
    }
    function g(b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30) -> b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15 {
	b1 := 1
	b2 := 2
	b15 := 15
	sstore(b16, b30)
    }

}
// ----
// step: stackLimitEvader
//
// {
//     {
//         mstore(0x40, memoryguard(0x0260))
//         let a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 := g(16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30)
//         sstore(0, 1)
//         a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 := g(16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30)
//     }
//     function g(b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30) -> b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15
//     {
//         mstore(0x0240, b16)
//         mstore(0x0220, b17)
//         mstore(0x0200, b18)
//         mstore(0x01e0, b19)
//         mstore(0x01c0, b20)
//         mstore(0x01a0, b21)
//         mstore(0x0180, b22)
//         mstore(0x0160, b23)
//         mstore(0x0140, b24)
//         mstore(0x0120, b25)
//         mstore(0x0100, b26)
//         mstore(0xe0, b27)
//         mstore(0xc0, b28)
//         mstore(0xa0, b29)
//         mstore(0x80, b30)
//         b1 := 1
//         b2 := 2
//         b15 := 15
//         sstore(mload(0x0240), mload(0x80))
//     }
// }
