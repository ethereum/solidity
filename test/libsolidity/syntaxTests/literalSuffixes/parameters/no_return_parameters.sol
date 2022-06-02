function nullSuffix(uint) pure {}

contract C {
    function f() public pure {
        return 1 nullSuffix;
    }
}
