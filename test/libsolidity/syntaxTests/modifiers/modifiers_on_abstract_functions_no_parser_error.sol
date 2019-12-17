// Previous versions of Solidity turned this
// into a parser error (they wrongly recognized
// these functions as state variables of
// function type).
abstract contract C
{
    modifier only_owner() { _; }
    function foo() only_owner public virtual;
    function bar() public only_owner virtual;
}
// ----
// SyntaxError: (212-253): Functions without implementation cannot have modifiers.
// SyntaxError: (258-299): Functions without implementation cannot have modifiers.
