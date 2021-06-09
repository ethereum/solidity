contract C
{
        function iWillRevertLevel1() pure public { revert(); }
        function iWillRevert() pure public { iWillRevertLevel1(); }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

