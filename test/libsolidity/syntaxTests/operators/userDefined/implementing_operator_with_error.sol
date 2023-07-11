type Int is int16;

using {IntError as +} for Int global;

error IntError(Int a, Int b);
// ----
// TypeError 8187: (27-35): Expected function name.
