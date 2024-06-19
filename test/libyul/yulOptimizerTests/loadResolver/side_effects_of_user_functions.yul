{
    function stores() { mstore(0, 1) }
    function reads() { sstore(9, mload(7)) }

    mstore(2, 9)
    reads()
    sstore(0, mload(2))
    stores()
    sstore(0, mload(2))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 9
//         let _2 := 2
//         mstore(_2, _1)
//         reads()
//         let _3 := _1
//         let _4 := 0
//         sstore(_4, _3)
//         stores()
//         sstore(_4, mload(_2))
//     }
//     function stores()
//     { mstore(0, 1) }
//     function reads()
//     { sstore(9, mload(7)) }
// }
