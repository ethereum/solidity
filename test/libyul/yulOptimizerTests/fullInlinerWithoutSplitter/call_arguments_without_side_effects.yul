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
// step: fullInlinerWithoutSplitter
//
// {
//     {
//         let a_2 := 111
//         let b_3 := 222
//         let c_4 := 333
//         let x := 111
//         let y := 222
//         let z := 333
//         let a_5 := x
//         let b_6 := y
//         let c_7 := z
//         let a_8 := mload(111)
//         let b_9 := sload(222)
//         let c_10 := calldataload(333)
//         let a_1 := 222
//         let a_11 := 111
//         let b_12 := a_1
//         let c_13 := mload(333)
//     }
//     function empty(a, b, c)
//     { }
// }
