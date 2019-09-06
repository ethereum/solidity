contract test {
    function () external payable {}
}
// ----
//     :test
// [
//   {
//     "stateMutability": "payable",
//     "type": "fallback"
//   }
// ]
