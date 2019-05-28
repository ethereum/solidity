pragma solidity >=0.0.0;

contract Error3 {
	constructor() public {
	    balances[tx.origin] = ; // missing RHS.
	}

}
// ----
// ParserError: (95-96): Expected primary expression.
// Warning: (95-96): Recovered in Statement at ';'.
