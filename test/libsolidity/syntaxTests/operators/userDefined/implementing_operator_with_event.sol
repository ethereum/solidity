type Int is int16;

using {C.IntEvent as +} for Int global;

contract C {
    event IntEvent(Int a, Int b);
}
// ----
// TypeError 8187: (27-37): Expected function name.
