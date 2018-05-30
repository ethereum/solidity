contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test() public {
        a = ActionChoices.GoStraight;
    }
    uint256 a;
}
// ----
// Warning: (80-148): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (117-141): Type enum test.ActionChoices is not implicitly convertible to expected type uint256.
