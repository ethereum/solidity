{
    mstore(0, 10)
    /// >>> int.from_bytes(web3.Web3.keccak(int.to_bytes(10, 32, byteorder='big')), byteorder='big')
    /// 89717814153306320011181716697424560163256864414616650038987186496166826726056
    let val := keccak256(0, 32)
    sstore(0, val)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         mstore(0, 10)
//         sstore(0, keccak256(0, 32))
//     }
// }
