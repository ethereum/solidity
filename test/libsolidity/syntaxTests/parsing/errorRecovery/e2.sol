pragma solidity >=0.0.0;

// Example to show why deleting the token at the
// point of error is bad. Here, ")" is missing
// and there is a ";" instead. That causes us to
// not be able to syncronize to ';'. Advance again and
// '}' is deleted and then we can't synchronize the contract.
contract Error2 {
	mapping (address => uint balances; // missing ) before "balances"
}
