contract test {
    function f() public {}
    function g() public payable {}
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs": [],
//     "name": "f",
//     "outputs": [],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "constant": false,
//     "inputs": [],
//     "name": "g",
//     "outputs": [],
//     "payable": true,
//     "stateMutability": "payable",
//     "type": "function"
//   }
// ]
