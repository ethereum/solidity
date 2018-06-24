contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test() public {
        choices = ActionChoices.RunAroundWavingYourHands;
    }
    ActionChoices choices;
}
// ----
// Warning: (80-168): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (123-161): Member "RunAroundWavingYourHands" not found or not visible after argument-dependent lookup in type(enum test.ActionChoices)
