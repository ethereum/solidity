contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test() public {
        a = 2;
        b = ActionChoices(a);
    }
    uint256 a;
    ActionChoices b;
}
// ----
// Warning: (80-155): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
