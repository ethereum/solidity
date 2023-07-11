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
//         let a_13 := 111
//         let b_14 := _2
//         let c_15 := _1
//         let x := 111
//         let y := 222
//         let z := 333
//         let a_16 := x
//         let b_17 := y
//         let c_18 := z
//         let _5 := calldataload(333)
//         let _7 := sload(222)
//         let a_19 := mload(111)
//         let b_20 := _7
//         let c_21 := _5
//         let a_1 := 222
//         let _11 := mload(333)
//         let a_22 := 111
//         let b_23 := a_1
//         let c_24 := _11
//     }
//     function empty(a, b, c)
//     { }
// }
