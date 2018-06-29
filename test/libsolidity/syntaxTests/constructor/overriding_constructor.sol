contract A { constructor() public {} }
contract B is A { function A() public pure returns (uint8) {} }
contract C is A { function A() public pure returns (uint8) {} }
contract D is B { function B() public pure returns (uint8) {} }
contract E is D { function B() public pure returns (uint8) {} }
// ----
