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
//     "inputs":
//     [
//       {
//         "internalType": "enum test.ActionChoices",
//         "name": "param",
//         "type": "uint8"
//       }
//     ],
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   },
//   {
//     "inputs": [],
//     "name": "ret",
//     "outputs":
//     [
//       {
//         "internalType": "enum test.ActionChoices",
//         "name": "",
//         "type": "uint8"
//       }
//     ],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
