contract X {}
library test {
    struct StructType { uint a; }
    function f(StructType storage b, uint[] storage c, X d) public returns (uint[] memory e, StructType storage f) { f = f; }
    function f1(uint[] memory c, X d) public pure returns (uint[] memory e) {  }
}
// ----
//     :X
// []
//
//
//     :test
// [
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256[]",
//         "name": "c",
//         "type": "uint256[]"
//       },
//       {
//         "internalType": "contract X",
//         "name": "d",
//         "type": "X"
//       }
//     ],
//     "name": "f1",
//     "outputs":
//     [
//       {
//         "internalType": "uint256[]",
//         "name": "e",
//         "type": "uint256[]"
//       }
//     ],
//     "stateMutability": "pure",
//     "type": "function"
//   }
// ]
