contract C
{
        function iWillRevertLevel2() pure public { revert(); }
        function iWillRevertLevel1() pure public { iWillRevertLevel2(); }
        function iWillRevert() pure public { iWillRevertLevel1(); }

        function test(bool _param) pure external returns(uint256)
        {
                if (_param) return 1;

                iWillRevert();
        }
}

