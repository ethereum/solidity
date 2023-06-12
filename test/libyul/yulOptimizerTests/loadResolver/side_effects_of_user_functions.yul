{
    function stores() { mstore(0, 1) }
    function reads() { sstore(9, mload(7)) }

    mstore(2, 9)
    reads()
    sstore(0, mload(2))
    stores()
    sstore(0, mload(2))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 9
//         let _2 := 2
//         mstore(_2, _1)
//         reads()
//         sstore(0, _1)
//         stores()
//         sstore(0, mload(_2))
//     }
//     function stores()
//     { mstore(0, 1) }
//     function reads()
//     { sstore(9, mload(7)) }
// }
