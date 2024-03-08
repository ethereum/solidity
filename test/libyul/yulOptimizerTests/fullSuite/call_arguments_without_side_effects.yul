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
//         mstore(1, 1)
//         mstore(2, 1)
//         mstore(3, 1)
//         mstore(4, 1)
//         mstore(5, 1)
//         mstore(6, 1)
//         let _1 := sload(8)
//         mstore(mload(7), 1)
//         mstore(_1, 1)
//         mstore(calldataload(9), 1)
//         let _2 := mload(13)
//         mstore(11, 1)
//         mstore(12, 1)
//         mstore(_2, 1)
//         return(0, 32)
//     }
// }
