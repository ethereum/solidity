contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test() public {
        choices = Sit;
    }
    ActionChoices choices;
}
// ----
// Warning: (80-133): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// DeclarationError: (123-126): Undeclared identifier.
