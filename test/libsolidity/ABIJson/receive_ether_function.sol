contract test {
    receive() external payable {}
}
// ----
//     :test
// [
//   {
//     "stateMutability": "payable",
//     "type": "receive"
//   }
// ]
