contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() {
        choices = Sit;
    }
    ActionChoices choices;
}
// ----
// DeclarationError 7576: (114-117): Undeclared identifier.
