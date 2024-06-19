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
//         let c_1 := _1
//         let b_1 := _2
//         let a_2 := _3
//         let x := 111
//         let y := 222
//         let c_2 := 333
//         let b_2 := y
//         let a_3 := x
//         let _4 := calldataload(333)
//         let _5 := sload(222)
//         let _6 := mload(111)
//         let c_3 := _4
//         let b_3 := _5
//         let a_4 := _6
//         let a_1 := 222
//         let _7 := mload(333)
//         let _8 := 111
//         let c_4 := _7
//         let b_4 := a_1
//         let a_5 := _8
//     }
//     function empty(a, b, c)
//     { }
// }
