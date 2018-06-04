contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test()
    {
        choices = ActionChoices.GoStraight;
    }
    ActionChoices choices;
}
// ----
// Warning: (80-151): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (80-151): No visibility specified. Defaulting to "public". 
