{
    function empty(a, b, c) {}

    // Constants
    empty(111, 222, 333)

    // Variables
    let x := 111
    let y := 222
    let z := 333
    empty(x, y, z)

    // Calls
    empty(mload(111), sload(222), calldataload(333))

    // Mix
    let a := 222
    empty(111, a, mload(333))
}
// ----
// step: fullInliner
//
// {
//     {
//         let _1 := 333
//         let _2 := 222
//         let _3 := 111
//         let c_13 := _1
//         let b_14 := _2
//         let a_15 := _3
//         let x := 111
//         let y := 222
//         let c_16 := 333
//         let b_17 := y
//         let a_18 := x
//         let _5 := calldataload(333)
//         let _7 := sload(222)
//         let _9 := mload(111)
//         let c_19 := _5
//         let b_20 := _7
//         let a_21 := _9
//         let a_1 := 222
//         let _11 := mload(333)
//         let _12 := 111
//         let c_22 := _11
//         let b_23 := a_1
//         let a_24 := _12
//     }
//     function empty(a, b, c)
//     { }
// }
