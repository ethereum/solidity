// An example with multiple errors.
// Most are caught by inserting an expected token.
// However some us S C Johnson recovery to
// skip over tokens.

pragma solidity >=0.0.0;

contract Error4 {
  constructor() public {
    balances[tx.origin] = 1 2; // missing operator
  }

  function sendCoin(address receiver, uint amount) public returns(bool sufficient) {
    if (balances[msg.sender] < amount) return false;
    balances[msg.sender] -= amount   // Missing ";"
    balances[receiver] += amount   // Another missing ";"
    emit Transfer(msg.sender  // truncated line
    return true;
  }


}
// ----
// ParserError: (249-250): Expected ';' but got 'Number'
// ParserError: (471-479): Expected ';' but got identifier
// ParserError: (529-533): Expected ';' but got 'emit'
// ParserError: (577-583): Expected ',' but got 'return'
// ParserError: (577-583): Expected primary expression.
// Warning: (588-589): Recovered in Statement at ';'.
