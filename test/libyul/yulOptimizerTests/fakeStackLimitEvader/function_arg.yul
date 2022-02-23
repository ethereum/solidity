{
	mstore(0x40, memoryguard(0))
	let $x := 0
	sstore(0, $x)
	function h($hx) -> y {
		y := $hx
	}
	sstore(1, h(32))
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0x40))
//     mstore(0x00, 0)
//     sstore(0, mload(0x00))
//     function h($hx) -> y
//     {
//         mstore(0x20, $hx)
//         y := mload(0x20)
//     }
//     sstore(1, h(32))
// }
