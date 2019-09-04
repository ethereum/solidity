contract test {
    function foo(uint a, uint b) public returns (uint d) { return a + b; }
    function boo(uint32 a) public pure returns (uint b) { return a * 4; }
}
// ----
//     :test
// [
//   {
//     "constant": true,
//     "inputs":
//     [
//       {
//         "internalType": "uint32",
//         "name": "a",
//         "type": "uint32"
//       }
//     ],
//     "name": "boo",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "pure",
//     "type": "function"
//   },
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "a",
//         "type": "uint256"
//       },
//       {
//         "internalType": "uint256",
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "foo",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "d",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
