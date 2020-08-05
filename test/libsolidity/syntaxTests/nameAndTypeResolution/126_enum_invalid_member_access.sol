contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() {
        choices = ActionChoices.RunAroundWavingYourHands;
    }
    ActionChoices choices;
}
// ----
// TypeError 9582: (114-152): Member "RunAroundWavingYourHands" not found or not visible after argument-dependent lookup in type(enum test.ActionChoices).
