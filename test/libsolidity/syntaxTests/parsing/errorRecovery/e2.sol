pragma solidity >=0.0.0;

// Example to show why deleting the token at the
// point of error in 0.5.8 is bad. Here, ")" is missing
// and there is a ";" instead. That causes us to
// not be able to synchronize to ';'. Advance again and
// '}' is deleted and then we can't synchronize the contract.
// There should be an an AST created this contract (with errors).
contract Error2 {
	mapping (address => uint balances; // missing ) before "balances"
}
// ----
// ParserError: (407-415): Expected ')' but got identifier
