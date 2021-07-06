abstract contract B
{
        function iWillRevert() pure public virtual { }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

contract C is B
{
        function iWillRevert() pure public override { revert(); }
}

// ----
// Warning 6321: (135-142): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
