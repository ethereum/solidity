contract test {
    fallback () external payable {}
}
// ----
//     :test
// [
//   {
//     "stateMutability": "payable",
//     "type": "fallback"
//   }
// ]
