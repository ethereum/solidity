contract C
{
    modifier only_owner() { _; }
    function foo() only_owner public;
    function bar() public only_owner;
}
// ----
// SyntaxError: (50-83): Functions without implementation cannot have modifiers.
// SyntaxError: (88-121): Functions without implementation cannot have modifiers.
