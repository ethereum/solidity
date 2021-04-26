// Test where the location of the memory offset is not known at compile time
// >>> import web3
// >>> asBytes = int(500).to_bytes(32, byteorder='big')
// >>> int.from_bytes(web3.Web3.keccak(asBytes), byteorder='big')
// 92647596584187651892918913434663110448935397770592030057655219009846081465370
// >>> int.from_bytes(web3.Web3.keccak(asBytes[:16]), byteorder='big')
// 110620294328144418057589324861608220015688365608948720310623173341503153578932
{
    let offset := calldataload(0)
    mstore(offset, 500)
    sstore(0, keccak256(offset, 32))
    sstore(1, keccak256(offset, 16))
}
// ----
// step: loadResolver
//
// {
//     let _1 := 0
//     mstore(calldataload(_1), 500)
//     sstore(_1, 92647596584187651892918913434663110448935397770592030057655219009846081465370)
//     sstore(1, 110620294328144418057589324861608220015688365608948720310623173341503153578932)
// }
