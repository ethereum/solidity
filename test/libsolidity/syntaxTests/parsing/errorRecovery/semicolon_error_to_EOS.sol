pragma solidity >=0.0.0;

// This is a contract with a missing semicolon for which there is
// recovery semicolon to synchronize up to.

// Tokens are deleted until the end of stream.

contract ErrorStatementToEOS {
  function five() returns(int) {
    uint256 a;
    a = 1
    b = 2
  }
}
// ----
// ParserError: (255-256): Expected ';' but got identifier. Cannot find ';' to synchronize to.
// ParserError: (267-267): Expected primary expression.
// ParserError: (267-267): Expected '}' but got end of source. Cannot find '}' to synchronize to.
