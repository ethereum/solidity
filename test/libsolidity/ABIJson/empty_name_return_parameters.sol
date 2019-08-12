contract test {
    function f(uint k) public returns (uint) {
        return k;
    }
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "k",
//         "type": "uint256"
//       }
//     ],
//     "name": "f",
//     "outputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "",
//         "type": "uint256"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
