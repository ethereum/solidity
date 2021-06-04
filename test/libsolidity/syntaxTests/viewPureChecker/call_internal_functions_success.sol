contract C {
    function g() pure public { g(); }
    function f() view public returns (uint) { f(); g(); }
    function h() public { h(); g(); f(); }
    function i() payable public { i(); h(); g(); f(); }
}
// ----
// Warning 5740: (102-105): Unreachable code.
// Warning 5740: (140-143): Unreachable code.
// Warning 5740: (145-148): Unreachable code.
// Warning 5740: (191-194): Unreachable code.
// Warning 5740: (196-199): Unreachable code.
// Warning 5740: (201-204): Unreachable code.
