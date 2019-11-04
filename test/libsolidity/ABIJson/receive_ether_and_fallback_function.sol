contract test {
    receive() external payable {}
    fallback() external {}
}
// ----
//     :test
// [
//   {
//     "stateMutability": "nonpayable",
//     "type": "fallback"
//   },
//   {
//     "stateMutability": "payable",
//     "type": "receive"
//   }
// ]
