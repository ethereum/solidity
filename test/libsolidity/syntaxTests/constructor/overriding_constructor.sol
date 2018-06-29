contract A { constructor() public {} }
contract B is A { function A() public pure returns (uint8) {} }
contract C is A { function A() public pure returns (uint8) {} }
contract D is B { function B() public pure returns (uint8) {} }
contract E is D { function B() public pure returns (uint8) {} }
// ----
// Warning: (57-100): This declaration shadows an existing declaration.
// Warning: (121-164): This declaration shadows an existing declaration.
// Warning: (185-228): This declaration shadows an existing declaration.
// Warning: (249-292): This declaration shadows an existing declaration.
