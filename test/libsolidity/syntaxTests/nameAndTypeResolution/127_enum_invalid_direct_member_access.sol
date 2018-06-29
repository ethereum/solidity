contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() public {
        choices = Sit;
    }
    ActionChoices choices;
}
// ----
// DeclarationError: (121-124): Undeclared identifier.
