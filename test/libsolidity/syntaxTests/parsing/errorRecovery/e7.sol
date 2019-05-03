pragma solidity >=0.0.0;

contract E7 {
	constructor() public {
	    balances[tx.origin] = // missing RHS and semicolon.
	}

}
