contract C {
    int immutable x = 1;
    int immutable y = 3;

    constructor() {
        x--;
        --x;
        y++;
        ++y;
        --y;
    }

    function f() public view returns (int, int) {
        return (x, y);
    }
}
// ----
// f() -> -1, 4
