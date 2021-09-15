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
// ====
// EVMVersion: >homestead
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := 1
//         let _2 := 15
//         let _3 := 10
//         let _4 := gt(not(gcd(_3, _2)), _1)
//         let _5 := gcd(_3, _2)
//         let _6 := not(0)
//         let _7 := lt(or(_1, add(gcd(_3, _2), _6)), _1)
//         let _8 := gcd(_3, _2)
//         let _9 := gcd(_3, _2)
//         pop(keccak256(gcd(_3, _2), or(gt(not(gcd(_3, _2)), _1), _1)))
//         mstore(lt(or(gt(_1, or(or(gt(or(or(or(gt(or(gt(_6, _9), _1), _8), _7), _5), _1), _1), _4), _1)), _1), _1), _1)
//         sstore(not(gcd(_3, _2)), _1)
//         sstore(0, 0)
//         sstore(2, _1)
//         extcodecopy(_1, msize(), _1, _1)
//         sstore(0, 0)
//         sstore(3, _1)
//     }
//     function gcd(_a, _b) -> out
//     {
//         switch _b
//         case 0 { out := _a }
//         default { out := gcd(_b, mod(_a, _b)) }
//     }
// }
