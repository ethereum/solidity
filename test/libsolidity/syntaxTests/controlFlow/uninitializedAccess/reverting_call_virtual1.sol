abstract contract B
{
        function iWillRevert() pure public virtual { }
}

contract C is B
{
        function iWillRevert() pure public override { revert(); }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

// ----
