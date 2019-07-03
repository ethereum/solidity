contract test {
    function foo(uint a, uint b) public returns (uint d) { return a + b; }
    function boo(uint32 a) public view returns(uint b) { return a * 4; }
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "name": "a",
//         "type": "uint256"
//       },
//       {
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "name": "foo",
//     "outputs":
//     [
//       {
//         "name": "d",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "constant": true,
//     "inputs":
//     [
//       {
//         "name": "a",
//         "type": "uint32"
//       }
//     ],
//     "name": "boo",
//     "outputs":
//     [
//       {
//         "name": "b",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "view",
//     "type": "function"
//   }
// ]
