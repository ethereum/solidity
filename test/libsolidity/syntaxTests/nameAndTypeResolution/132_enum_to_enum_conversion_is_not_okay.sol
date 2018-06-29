contract test {
    enum Paper { Up, Down, Left, Right }
    enum Ground { North, South, West, East }
    constructor() public {
        Ground(Paper.Up);
    }
}
// ----
// TypeError: (137-153): Explicit type conversion not allowed from "enum test.Paper" to "enum test.Ground".
