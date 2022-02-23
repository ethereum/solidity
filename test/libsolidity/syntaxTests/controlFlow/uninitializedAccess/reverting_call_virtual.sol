abstract contract B
{
        function iWillRevert() pure public virtual { revert(); }

        function test2(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

contract C is B
{
        function iWillRevert() pure public override {  }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

// ----
// Warning 6321: (146-153): Unnamed return variable can remain unassigned when the function is called when "C" is the most derived contract. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (381-388): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
