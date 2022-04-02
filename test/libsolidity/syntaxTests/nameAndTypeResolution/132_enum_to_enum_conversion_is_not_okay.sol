contract test {
    enum Paper { Up, Down, Left, Right }
    enum Ground { North, South, West, East }
    constructor() {
        Ground(Paper.Up);
    }
}
// ----
// TypeError 9640: (130-146='Ground(Paper.Up)'): Explicit type conversion not allowed from "enum test.Paper" to "enum test.Ground".
