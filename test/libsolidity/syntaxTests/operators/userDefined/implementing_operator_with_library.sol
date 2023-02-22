type Int is int16;

using {L as +} for Int global;

library L {}
// ----
// TypeError 8187: (27-28): Expected function name.
