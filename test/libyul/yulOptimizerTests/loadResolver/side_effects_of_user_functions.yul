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
// step: loadResolver
// ----
// {
//     function stores()
//     { mstore(0, 1) }
//     function reads()
//     { sstore(9, mload(7)) }
//     let _6 := 9
//     let _7 := 2
//     mstore(_7, _6)
//     reads()
//     let _9 := _6
//     let _10 := 0
//     sstore(_10, _9)
//     stores()
//     sstore(_10, mload(_7))
// }
