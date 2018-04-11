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
// Warning: (203-236): Modifiers of functions without implementation are ignored.
// Warning: (241-274): Modifiers of functions without implementation are ignored.
