contract test {
    function() external {}
}
// ----
//     :test
// [
//   {
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "fallback"
//   }
// ]
