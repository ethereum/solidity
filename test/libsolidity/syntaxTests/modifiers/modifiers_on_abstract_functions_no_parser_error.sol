// Previous versions of Solidity turned this
// into a parser error (they wrongly recognized
// these functions as state variables of
// function type).
abstract contract C
{
    modifier only_owner() { _; }
    function foo() only_owner public;
    function bar() public only_owner;
}
// ----
// SyntaxError: (212-245): Functions without implementation cannot have modifiers.
// SyntaxError: (250-283): Functions without implementation cannot have modifiers.
