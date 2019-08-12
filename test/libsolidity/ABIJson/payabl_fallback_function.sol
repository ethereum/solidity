contract test {
    function () external payable {}
}
// ----
//     :test
// [
//   {
//     "payable": true,
//     "stateMutability": "payable",
//     "type": "fallback"
//   }
// ]
