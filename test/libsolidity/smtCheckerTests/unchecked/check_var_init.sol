pragma experimental SMTChecker;

contract C {
    uint public x = msg.value - 10; // can underflow
    constructor() payable {}
}

contract D {
	function h() internal returns (uint) {
		return msg.value - 10; // can underflow
	}
    function f() public {
        unchecked {
            h(); // unchecked here does not mean h does not underflow
        }
    }
}
// ----
// Warning 3944: (66-80): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()
// Warning 3944: (193-207): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\n\n\nTransaction trace:\nD.constructor()\nD.f()\n    D.h() -- internal call
