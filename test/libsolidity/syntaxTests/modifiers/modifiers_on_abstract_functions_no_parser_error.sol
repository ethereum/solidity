// Previous versions of Solidity turned this
// into a parser error (they wrongly recognized
// these functions as state variables of
// function type).
contract C
{
    modifier only_owner() { _; }
    function foo() only_owner public;
    function bar() public only_owner;
}
// ----
// SyntaxError: (203-236): Functions without implementation cannot have modifiers.
// SyntaxError: (241-274): Functions without implementation cannot have modifiers.
// TypeError: (153-276): Contract "C" should be marked as abstract.
