library test {
    struct StructType { uint a; }
    function f(StructType storage b, uint[] storage c, test d) public returns (uint[] memory e, StructType storage f) { f = f; }
    function f1(uint[] memory c, test d) public pure returns (uint[] memory e) {  }
}
// ----
//     :test
// [
//   {
//     "constant": true,
//     "inputs":
//     [
//       {
//         "internalType": "uint256[]",
//         "name": "c",
//         "type": "uint256[]"
//       },
//       {
//         "internalType": "library test",
//         "name": "d",
//         "type": "test"
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
//     "payable": false,
//     "stateMutability": "pure",
//     "type": "function"
//   }
// ]
