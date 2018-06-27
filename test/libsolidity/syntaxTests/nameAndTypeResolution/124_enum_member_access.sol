contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor()
    {
        choices = ActionChoices.GoStraight;
    }
    ActionChoices choices;
}
// ----
// Warning: (80-149): No visibility specified. Defaulting to "public". 
