contract test {
    function f() public {}
    function g() public payable {}
}
// ----
//     :test
// [
//   {
//     "inputs": [],
//     "name": "f",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs": [],
//     "name": "g",
//     "outputs": [],
//     "stateMutability": "payable",
//     "type": "function"
//   }
// ]
