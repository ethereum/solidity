contract test {
    fallback() external {}
}
// ----
//     :test
// [
//   {
//     "stateMutability": "nonpayable",
//     "type": "fallback"
//   }
// ]
