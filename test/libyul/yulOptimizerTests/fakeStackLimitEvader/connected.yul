{
	mstore(0x40, memoryguard(0))
	function g() -> a, b {
		a := 21
		let $c := 1
		b,a,$c := z()
	}
	function f() -> x {
		let $x2
		$x2 := 42
		let $x3, $x4 := g()
		x := mul(add($x2, $x3), h($x4))
		sstore($x3, $x4)
	}
	function h(v) -> a {
		let x, $z, y := z()
		a, $z, v := z()
	}
	function z() -> a,b,c { let $x := 0 }
	sstore(0, f())
	let x, y := g()
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0xa0))
//     function g() -> a, b
//     {
//         a := 21
//         mstore(0x60, 1)
//         let b_8, a_9, $c_10 := z()
//         mstore(0x60, $c_10)
//         a := a_9
//         b := b_8
//     }
//     function f() -> x
//     {
//         let $x2_11
//         mstore(0x20, $x2_11)
//         mstore(0x20, 42)
//         let $x3_13, $x4_14 := g()
//         mstore(0x00, $x4_14)
//         mstore(0x40, $x3_13)
//         x := mul(add(mload(0x20), mload(0x40)), h(mload(0x00)))
//         sstore(mload(0x40), mload(0x00))
//     }
//     function h(v) -> a_1
//     {
//         let x_2_15, $z_16, y_17 := z()
//         mstore(0x60, $z_16)
//         let y := y_17
//         let x_2 := x_2_15
//         let a_1_18, $z_19, v_20 := z()
//         mstore(0x60, $z_19)
//         v := v_20
//         a_1 := a_1_18
//     }
//     function z() -> a_3, b_4, c
//     { mstore(0x80, 0) }
//     sstore(0, f())
//     let x_5, y_6 := g()
// }
