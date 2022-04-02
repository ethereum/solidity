contract B {
    function f() external {}
    function g() internal {}
}
contract C is B {
    function i() public {
        B.f();
        B.g.selector;
    }
}
// ----
// TypeError 3419: (125-130='B.f()'): Cannot call function via contract type name.
// TypeError 9582: (140-152='B.g.selector'): Member "selector" not found or not visible after argument-dependent lookup in function ().
