contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() public
    {
        choices = ActionChoices.GoStraight;
    }
    ActionChoices choices;
}
