pragma experimental SMTChecker;
contract C {
    function f(function(uint) external g) public {
    }
}
