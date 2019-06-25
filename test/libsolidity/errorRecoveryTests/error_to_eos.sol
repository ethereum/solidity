// Example which where scanning hits EOS, so we reset.
// Here we recover in the contractDefinition.
// There should be an an AST created this contract (with errors).
contract Error2 {
  mapping (address => uint balances) // missing ;
}

// There is no error in this contract
contract SendCoin {
  function sendCoin(address receiver, uint amount) public returns(bool sufficient) {
    if (balances[msg.sender] < amount) return false;
    balances[msg.sender] -= amount;
    balances[receiver] += amount;
    emit Transfer(msg.sender, receiver, amount);
    return true;
  }
}

// ----
// ParserError: (212-220): Expected ')' but got identifier
// ParserError: (220-221): Expected ';' but got ')'
// ParserError: (220-221): Function, variable, struct or modifier declaration expected.
// Warning: (235-236): Recovered in ContractDefinition at '}'.
