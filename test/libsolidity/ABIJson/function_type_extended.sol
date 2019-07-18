contract test {
    function g(function(test) external returns (test[] memory) x) public {}
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "internalType": "function (contract test) external returns (contract test[])",
//         "name": "x",
//         "type": "function"
//       }
//     ],
//     "name": "g",
//     "outputs": [],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
