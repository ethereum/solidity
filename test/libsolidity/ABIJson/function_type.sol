contract test {
    function g(function(uint) external returns (uint) x) public {}
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "internalType": "function (uint256) external returns (uint256)",
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
