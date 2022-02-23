pragma solidity >=0.0.0;

contract Error3 {
	constructor() {
	    balances[tx.origin] = ; // missing RHS.
	}

}
// ----
// ParserError 6933: (88-89): Expected primary expression.
// Warning 3347: (88-89): Recovered in Statement at ';'.
