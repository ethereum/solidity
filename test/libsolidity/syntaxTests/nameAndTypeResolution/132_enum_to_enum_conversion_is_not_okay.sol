contract test {
    enum Paper { Up, Down, Left, Right }
    enum Ground { North, South, West, East }
    function test() public {
        Ground(Paper.Up);
    }
}
// ----
// Warning: (106-162): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (139-155): Explicit type conversion not allowed from "enum test.Paper" to "enum test.Ground".
