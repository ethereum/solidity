{
	function g_(x) -> z_ {
		if calldataload(1) { z_ := g_(x) }
		sstore(z_, calldataload(add(x, 1)))
	}
	function datasize_(x) -> linkersymbol_ {
		if calldataload(0) { linkersymbol_ := datasize_(x) }
		sstore(linkersymbol_, calldataload(linkersymbol_))
	}
	let dataoffset_ := datasize_(7)
	let x_ := g_(9)
	sstore(dataoffset_, x_)
}
// ----
// step: fullSuite
//
// {
//     {
//         let linkersymbol_1 := 0
//         let _1 := calldataload(linkersymbol_1)
//         if _1
//         {
//             let linkersymbol_2 := linkersymbol_1
//             if _1
//             {
//                 let linkersymbol_3 := linkersymbol_1
//                 if _1
//                 {
//                     let linkersymbol_4 := linkersymbol_1
//                     if _1
//                     {
//                         let linkersymbol_5 := linkersymbol_1
//                         if _1
//                         {
//                             linkersymbol_5 := datasize_1042()
//                         }
//                         sstore(linkersymbol_5, calldataload(linkersymbol_5))
//                         linkersymbol_4 := linkersymbol_5
//                     }
//                     sstore(linkersymbol_4, calldataload(linkersymbol_4))
//                     linkersymbol_3 := linkersymbol_4
//                 }
//                 sstore(linkersymbol_3, calldataload(linkersymbol_3))
//                 linkersymbol_2 := linkersymbol_3
//             }
//             sstore(linkersymbol_2, calldataload(linkersymbol_2))
//             linkersymbol_1 := linkersymbol_2
//         }
//         sstore(linkersymbol_1, calldataload(linkersymbol_1))
//         let z := 0
//         let _2 := calldataload(1)
//         if _2
//         {
//             let z_1 := z
//             if _2
//             {
//                 let z_2 := z
//                 if _2
//                 {
//                     let z_3 := z
//                     if _2 { z_3 := g_780() }
//                     sstore(z_3, calldataload(10))
//                     z_2 := z_3
//                 }
//                 sstore(z_2, calldataload(10))
//                 z_1 := z_2
//             }
//             sstore(z_1, calldataload(10))
//             z := z_1
//         }
//         sstore(z, calldataload(10))
//         sstore(linkersymbol_1, z)
//     }
//     function g_780() -> z
//     {
//         let _1 := calldataload(1)
//         if _1
//         {
//             let z_1 := z
//             if _1
//             {
//                 let z_2 := z
//                 if _1
//                 {
//                     let z_3 := z
//                     if _1
//                     {
//                         let z_4 := z
//                         if _1
//                         {
//                             let z_5 := z
//                             if _1
//                             {
//                                 let z_6 := z
//                                 if _1 { z_6 := g_3303() }
//                                 sstore(z_6, calldataload(10))
//                                 z_5 := z_6
//                             }
//                             sstore(z_5, calldataload(10))
//                             z_4 := z_5
//                         }
//                         sstore(z_4, calldataload(10))
//                         z_3 := z_4
//                     }
//                     sstore(z_3, calldataload(10))
//                     z_2 := z_3
//                 }
//                 sstore(z_2, calldataload(10))
//                 z_1 := z_2
//             }
//             sstore(z_1, calldataload(10))
//             z := z_1
//         }
//         sstore(z, calldataload(10))
//     }
//     function g_3303() -> z
//     {
//         if calldataload(1) { z := g(9) }
//         sstore(z, calldataload(add(9, 1)))
//     }
//     function g(x) -> z
//     {
//         if calldataload(1) { z := g(x) }
//         sstore(z, calldataload(add(x, 1)))
//     }
//     function datasize_1042() -> linkersymbol_1
//     {
//         let _1 := calldataload(linkersymbol_1)
//         if _1
//         {
//             let linkersymbol_2 := linkersymbol_1
//             if _1
//             {
//                 let linkersymbol_3 := linkersymbol_1
//                 if _1
//                 {
//                     let linkersymbol_4 := linkersymbol_1
//                     if _1
//                     {
//                         let linkersymbol_5 := linkersymbol_1
//                         if _1
//                         {
//                             let linkersymbol_6 := linkersymbol_1
//                             if _1
//                             {
//                                 linkersymbol_6 := datasize_3305()
//                             }
//                             sstore(linkersymbol_6, calldataload(linkersymbol_6))
//                             linkersymbol_5 := linkersymbol_6
//                         }
//                         sstore(linkersymbol_5, calldataload(linkersymbol_5))
//                         linkersymbol_4 := linkersymbol_5
//                     }
//                     sstore(linkersymbol_4, calldataload(linkersymbol_4))
//                     linkersymbol_3 := linkersymbol_4
//                 }
//                 sstore(linkersymbol_3, calldataload(linkersymbol_3))
//                 linkersymbol_2 := linkersymbol_3
//             }
//             sstore(linkersymbol_2, calldataload(linkersymbol_2))
//             linkersymbol_1 := linkersymbol_2
//         }
//         sstore(linkersymbol_1, calldataload(linkersymbol_1))
//     }
//     function datasize_3305() -> linkersymbol_1
//     {
//         if calldataload(linkersymbol_1)
//         {
//             linkersymbol_1 := datasize_(7)
//         }
//         sstore(linkersymbol_1, calldataload(linkersymbol_1))
//     }
//     function datasize_(x) -> linkersymbol_
//     {
//         if calldataload(linkersymbol_) { linkersymbol_ := datasize_(x) }
//         sstore(linkersymbol_, calldataload(linkersymbol_))
//     }
// }
