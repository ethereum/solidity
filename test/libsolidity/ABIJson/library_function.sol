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
//         "name": "c",
//         "type": "uint256[]"
//       },
//       {
//         "name": "d",
//         "type": "test"
//       }
//     ],
//     "name": "f1",
//     "outputs":
//     [
//       {
//         "name": "e",
//         "type": "uint256[]"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "pure",
//     "type": "function"
//   }
// ]
