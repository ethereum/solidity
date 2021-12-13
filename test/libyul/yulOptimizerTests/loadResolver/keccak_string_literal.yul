{
    let converted := 14
    let _1 := 50
    mstore(_1, "abcdefghijklmn")
    // >>> int.from_bytes(web3.Web3.keccak(text="abcdefghijklmn"), byteorder='big')
    // 51246744213555520563123611275127692828770413530219146609532820042079541949502

    sstore(0, keccak256(_1, converted))
    sstore(0, add(mload(_1), 1))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 50
//         let _2 := "abcdefghijklmn"
//         mstore(_1, _2)
//         let _3 := 51246744213555520563123611275127692828770413530219146609532820042079541949502
//         let _4 := 0
//         sstore(_4, _3)
//         sstore(_4, add(_2, 1))
//     }
// }
