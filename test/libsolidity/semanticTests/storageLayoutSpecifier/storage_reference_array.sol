contract C layout at 42 {
    uint[] public array;
    function initUsingReference() public {
        uint[] storage ptr = array;
        for(uint i = 0; i < 10; ++i)
            ptr.push(i + 1);

        validate(array);
    }
    function validate(uint[] memory ptr) public pure {
        for(uint i = 0; i < 10; ++i)
            require(ptr[i] == i + 1);
    }
}
// ----
// initUsingReference() ->
// gas irOptimized: 273556
// gas legacy: 274795
// gas legacyOptimized: 271954
// gas ssaCFGOptimized: 273428
// array(uint256): 0 -> 1
// array(uint256): 9 -> 10
