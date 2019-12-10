contract test {
    function f(uint, uint k) public returns (uint ret_k, uint ret_g) {
        uint g = 8;
        ret_k = k;
        ret_g = g;
    }
}
// ----
//     :test
// [
//   {
//     "inputs":
//     [
//       {
//         "internalType": "uint256",
//         "name": "",
//         "type": "uint256"
//       },
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
//         "name": "ret_k",
//         "type": "uint256"
//       },
//       {
//         "internalType": "uint256",
//         "name": "ret_g",
//         "type": "uint256"
//       }
//     ],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
