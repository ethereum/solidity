abstract contract B
{
        function iWillRevert() pure public virtual { revert(); }
}

contract C is B
{
        function iWillRevert() pure public override {  }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                super.iWillRevert();
        }
}

// ----
