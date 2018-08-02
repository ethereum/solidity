pragma experimental SMTChecker;
contract C {
    function f() public pure { uint a = 2; assert(a == 2); }
}
