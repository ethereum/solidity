// bug #1801
contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor(ActionChoices param) public {}
    function ret() public returns (ActionChoices) {
        ActionChoices action = ActionChoices.GoLeft;
        return action;
    }
}
// ----
//     :test
// [
//   {
//     "constant": false,
//     "inputs": [],
//     "name": "ret",
//     "outputs":
//     [
//       {
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "function"
//   },
//   {
//     "inputs":
//     [
//       {
//         "name": "param",
//         "type": "uint8"
//       }
//     ],
//     "payable": false,
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   }
// ]
