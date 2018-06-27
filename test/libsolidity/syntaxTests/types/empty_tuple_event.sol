pragma solidity ^0.4.3;
contract C {
    event SomeEvent();
    function a() public {
        (emit SomeEvent(), 7);
    }
}
// ----
// ParserError: (95-99): Expected primary expression.
