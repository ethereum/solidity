{
    function empty(a, b, c) {
        mstore(a, 1)
        mstore(b, 1)
        mstore(c, 1)
    }

    // Constants
    empty(1, 2, 3)

    // Variables
    let x := 4
    let y := 5
    let z := 6
    empty(x, y, z)

    // Calls
    empty(mload(7), sload(8), calldataload(9))

    // Mix
    let a := 12
    empty(11, a, mload(13))

    return(0, 32)
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := 1
//         mstore(_1, _1)
//         mstore(2, _1)
//         mstore(3, _1)
//         mstore(4, _1)
//         mstore(5, _1)
//         mstore(6, _1)
//         let _2 := sload(8)
//         mstore(mload(7), _1)
//         mstore(_2, _1)
//         mstore(calldataload(9), _1)
//         let _3 := mload(13)
//         mstore(11, _1)
//         mstore(12, _1)
//         mstore(_3, _1)
//         return(0, 32)
//     }
// }
