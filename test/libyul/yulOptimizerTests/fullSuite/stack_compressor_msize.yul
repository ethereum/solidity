{
	let a

	function gcd(_a, _b) -> out
	{
		// GCD algorithm. in order to test underlying stack compressor this function must not be inlined.
		switch _b
		case 0 { out := _a }
		default { out := gcd(_b, mod(_a, _b)) }
	}

	function f() -> out
	{
		out := gcd(10, 15)
	}

	function foo_singlereturn_0() -> out
	{
		mstore(lt(or(gt(1,or(or(gt(or(or(or(1,gt(or(gt(or(or(keccak256(f(),or(gt(not(f()),1),1)),1),not(1)),f()),1),f())),lt(or(1,sub(f(),1)),1)),f()),1),1),gt(not(f()),1))),1),1),1)
		sstore(not(f()),1)
	}

	function foo_singlereturn_1(in_1, in_2) -> out
	{
		extcodecopy(1,msize(),1,1)
	}

	a := foo_singlereturn_0()
	sstore(0,0)
	sstore(2,1)

	a := foo_singlereturn_1(calldataload(0),calldataload(3))
	sstore(0,0)
	sstore(3,1)
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := gt(not(gcd_1042()), 1)
//         let _2 := gcd_1043()
//         let _3 := not(0)
//         let _4 := lt(or(1, add(gcd_1044(), _3)), 1)
//         let _5 := gcd_1045()
//         let _6 := gcd_1046()
//         pop(keccak256(gcd_1048(), or(gt(not(gcd_1047()), 1), 1)))
//         mstore(lt(or(gt(1, or(or(gt(or(or(or(gt(or(gt(_3, _6), 1), _5), _4), _2), 1), 1), _1), 1)), 1), 1), 1)
//         sstore(not(gcd_1049()), 1)
//         sstore(0, 0)
//         sstore(2, 1)
//         extcodecopy(1, msize(), 1, 1)
//         sstore(0, 0)
//         sstore(3, 1)
//     }
//     function gcd_1042() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1043() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1044() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1045() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1046() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1047() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1048() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd_1049() -> out
//     {
//         switch 5
//         case 0 { out := 10 }
//         default { out := gcd(5, mod(10, 5)) }
//     }
//     function gcd(_a, _b) -> out
//     {
//         switch _b
//         case 0 { out := _a }
//         default { out := gcd(_b, mod(_a, _b)) }
//     }
// }
