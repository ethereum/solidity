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
//         let b_2, a_3, $c := z()
//         mstore(0x60, $c)
//         a := a_3
//         b := b_2
//     }
//     function f() -> x
//     {
//         mstore(0x40, 0)
//         mstore(0x40, 42)
//         let $x3, $x4 := g()
//         mstore(0x00, $x4)
//         mstore(0x20, $x3)
//         x := mul(add(mload(0x40), mload(0x20)), h(mload(0x00)))
//         sstore(mload(0x20), mload(0x00))
//     }
//     function h(v) -> a_1
//     {
//         let x_3, $z, y_2 := z()
//         mstore(0x60, $z)
//         let y := y_2
//         let x_1 := x_3
//         let a_4, $z_1, v_1 := z()
//         mstore(0x60, $z_1)
//         v := v_1
//         a_1 := a_4
//     }
//     function z() -> a_2, b_1, c
//     { mstore(0x80, 0) }
//     sstore(0, f())
//     let x_2, y_1 := g()
// }
