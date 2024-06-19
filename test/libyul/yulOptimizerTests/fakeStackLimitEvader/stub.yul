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
//         mstore(0x60, 0)
//         mstore(0x40, 42)
//         sstore(mload(0x60), mload(0x40))
//         mstore(0x60, 21)
//     }
//     function g(gx)
//     {
//         let $gx, $gy := tuple2()
//         mstore(0x40, $gy)
//         mstore(0x60, $gx)
//         {
//             let $gx_1, $gy_1 := tuple2()
//             mstore(0x40, $gy_1)
//             mstore(0x60, $gx_1)
//         }
//         {
//             let $gx_2, gx_1 := tuple2()
//             mstore(0x60, $gx_2)
//             gx := gx_1
//         }
//         {
//             let gx_2, $gy_2 := tuple2()
//             mstore(0x40, $gy_2)
//             gx := gx_2
//         }
//     }
//     function h(hx, hy, hz, hw)
//     {
//         let $hx, $hy, $hz, $hw := tuple4()
//         mstore(0x00, $hw)
//         mstore(0x20, $hz)
//         mstore(0x40, $hy)
//         mstore(0x60, $hx)
//         {
//             let hx_1, $hy_1, hz_1, $hw_1 := tuple4()
//             mstore(0x00, $hw_1)
//             mstore(0x40, $hy_1)
//             hz := hz_1
//             hx := hx_1
//         }
//         {
//             let $hx_1, $hy_2, hz_2, hw_1 := tuple4()
//             mstore(0x40, $hy_2)
//             mstore(0x60, $hx_1)
//             hw := hw_1
//             hz := hz_2
//         }
//     }
//     function tuple2() -> a, b
//     { }
//     function tuple4() -> a_1, b_1, c, d
//     { }
//     f()
//     g(0)
//     h(1, 2, 3, 4)
// }
