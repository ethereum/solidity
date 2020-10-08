contract First {
    function fun() public returns (bool) {
        return Second(1).fun(1, true, 3) > 0;
    }
}
contract Second {
    function fun(uint, bool, uint) public returns (uint) {
        if (First(2).fun() == true) return 1;
    }
}
// ----
// Warning 6321: (183-187): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
