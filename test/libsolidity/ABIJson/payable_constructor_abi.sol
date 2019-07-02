contract test {
    constructor(uint param1, test param2, bool param3) public payable {}
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
//     "payable": true,
//     "stateMutability": "payable",
//     "type": "constructor"
//   }
// ]
