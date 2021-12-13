// This test used to crash: https://github.com/ethereum/solidity/issues/11801
{
  for {} addmod(keccak256(0x0,create(0x0, 0x0, 0x0)), 0x0, 0x0) {} {}
}
// ----
// step: loadResolver
//
// {
//     {
//         for { }
//         addmod(keccak256(0x0, create(0x0, 0x0, 0x0)), 0x0, 0x0)
//         { }
//         { }
//     }
// }
