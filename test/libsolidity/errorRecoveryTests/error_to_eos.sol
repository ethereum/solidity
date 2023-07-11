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
// ParserError 6635: (235-236): Expected identifier but got '}'
// ParserError 6635: (276-284): Expected ';' but got 'contract'
// ParserError 9182: (276-284): Function, variable, struct or modifier declaration expected.
// Warning 3796: (572-573): Recovered in ContractDefinition at '}'.
// ParserError 7858: (574-575): Expected pragma, import directive or contract/interface/library/struct/enum/constant/function/error definition.
