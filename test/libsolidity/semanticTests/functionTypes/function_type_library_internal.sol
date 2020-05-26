library Utils {
    function reduce(
        uint256[] memory array,
        function(uint, uint) internal returns (uint) f,
        uint256 init
    ) internal returns (uint256) {
        for (uint256 i = 0; i < array.length; i++) {
            init = f(array[i], init);
        }
        return init;
    }

    function sum(uint256 a, uint256 b) internal returns (uint256) {
        return a + b;
    }
}


contract C {
    function f(uint256[] memory x) public returns (uint256) {
        return Utils.reduce(x, Utils.sum, 0);
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256[]): 0x20, 0x3, 0x1, 0x7, 0x3 -> 11
