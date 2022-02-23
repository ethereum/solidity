contract C
{
        function iWillRevert() pure public { revert(); }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

