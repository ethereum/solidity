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
//         let c_1 := 333
//         let b_2 := 222
//         let a_3 := 111
//         let x := 111
//         let y := 222
//         let z := 333
//         let c_4 := z
//         let b_5 := y
//         let a_6 := x
//         let c_7 := calldataload(333)
//         let b_8 := sload(222)
//         let a_9 := mload(111)
//         let a_1 := 222
//         let c_10 := mload(333)
//         let b_11 := a_1
//         let a_12 := 111
//     }
//     function empty(a, b, c)
//     { }
// }
