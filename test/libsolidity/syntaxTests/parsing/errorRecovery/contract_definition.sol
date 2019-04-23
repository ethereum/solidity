pragma solidity >=0.0.0;

contract {
	mapping (address => uint) balances;
}

contract One {
	mapping2 (address => uint) balances;
}

library Address {
    function (address account internal view returns (bool) {
        return account > 0;
    }
}

// ----
// ParserError: (12-13): Expected identifier but got '{'
// ParserError: (12-13): Expected '}' but got '{'. Skipping to next '}'.
// ParserError: (79-80): Expected identifier but got '('
// ParserError: (79-80): Expected '}' but got '('. Skipping to next '}'.
// ParserError: (158-166): Expected ',' but got 'internal'
// ParserError: (158-166): Expected '}' but got 'internal'. Skipping to next '}'.
// ParserError: (223-224): Expected pragma, import directive or contract/interface/library definition.
