contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() public {
        choices = ActionChoices.RunAroundWavingYourHands;
    }
    ActionChoices choices;
}
// ----
// TypeError: (121-159): Member "RunAroundWavingYourHands" not found or not visible after argument-dependent lookup in type(enum test.ActionChoices).
