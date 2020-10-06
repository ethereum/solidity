{
	mstore(0x40, memoryguard(0))
	function f() {
	    let $fx
	    let $fy := 42
	    sstore($fx, $fy)
	    $fx := 21
	}
	function g(gx) {
	    let $gx, $gy := tuple2()
	    { $gx, $gy := tuple2() }
	    { $gx, gx := tuple2() }
	    { gx, $gy := tuple2() }
	}
	function h(hx, hy, hz, hw) {
	    let $hx, $hy, $hz, $hw := tuple4()
	    { hx, $hy, hz, $hw := tuple4() }
	    { $hx, $hy, hz, hw := tuple4() }
	}
	function tuple2() -> a, b {}
	function tuple4() -> a, b, c, d {}
    f()
    g(0)
    h(1, 2, 3, 4)
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0x80))
//     function f()
//     {
//         let $fx_8
//         mstore(0x40, $fx_8)
//         mstore(0x60, 42)
//         sstore(mload(0x40), mload(0x60))
//         mstore(0x40, 21)
//     }
//     function g(gx)
//     {
//         let $gx_11, $gy_12 := tuple2()
//         mstore(0x40, $gy_12)
//         mstore(0x60, $gx_11)
//         {
//             let $gx_13, $gy_14 := tuple2()
//             mstore(0x40, $gy_14)
//             mstore(0x60, $gx_13)
//         }
//         {
//             let $gx_15, gx_16 := tuple2()
//             mstore(0x60, $gx_15)
//             gx := gx_16
//         }
//         {
//             let gx_17, $gy_18 := tuple2()
//             mstore(0x40, $gy_18)
//             gx := gx_17
//         }
//     }
//     function h(hx, hy, hz, hw)
//     {
//         let $hx_19, $hy_20, $hz_21, $hw_22 := tuple4()
//         mstore(0x00, $hw_22)
//         mstore(0x60, $hz_21)
//         mstore(0x40, $hy_20)
//         mstore(0x20, $hx_19)
//         {
//             let hx_23, $hy_24, hz_25, $hw_26 := tuple4()
//             mstore(0x00, $hw_26)
//             mstore(0x40, $hy_24)
//             hz := hz_25
//             hx := hx_23
//         }
//         {
//             let $hx_27, $hy_28, hz_29, hw_30 := tuple4()
//             mstore(0x40, $hy_28)
//             mstore(0x20, $hx_27)
//             hw := hw_30
//             hz := hz_29
//         }
//     }
//     function tuple2() -> a, b
//     { }
//     function tuple4() -> a_1, b_2, c, d
//     { }
//     f()
//     g(0)
//     h(1, 2, 3, 4)
// }
