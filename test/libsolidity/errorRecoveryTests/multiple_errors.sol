// An example with multiple errors.
// Most are caught by inserting an expected token.
// However some us S C Johnson recovery to
// skip over tokens.

pragma solidity >=0.0.0;

contract Error4 {
  constructor() {
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
// ParserError 6635: (242-243): Expected ';' but got 'Number'
// ParserError 6635: (464-472): Expected ';' but got identifier
// ParserError 6635: (522-526): Expected ';' but got 'emit'
// ParserError 6635: (570-576): Expected ',' but got 'return'
// ParserError 6933: (570-576): Expected primary expression.
// Warning 3796: (581-582): Recovered in Statement at ';'.
