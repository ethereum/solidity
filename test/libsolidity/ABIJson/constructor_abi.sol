contract test {
    constructor(uint param1, test param2, bool param3) public {}
}
// ----
//     :test
// [
//   {
//     "inputs":
//     [
//       {
//         "name": "param1",
//         "type": "uint256"
//       },
//       {
//         "name": "param2",
//         "type": "address"
//       },
//       {
//         "name": "param3",
//         "type": "bool"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   }
// ]
