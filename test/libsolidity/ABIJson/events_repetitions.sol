library L {
	event e();
}
contract C {
	constructor() {
		emit L.e();
	}
	function f() public {
		emit L.e();
	}
}

// ----
//     :C
// [
//   {
//     "inputs": [],
//     "stateMutability": "nonpayable",
//     "type": "constructor"
//   },
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e",
//     "type": "event"
//   },
//   {
//     "inputs": [],
//     "name": "f",
//     "outputs": [],
//     "stateMutability": "nonpayable",
//     "type": "function"
//   }
// ]
//
//
//     :L
// [
//   {
//     "anonymous": false,
//     "inputs": [],
//     "name": "e",
//     "type": "event"
//   }
// ]
