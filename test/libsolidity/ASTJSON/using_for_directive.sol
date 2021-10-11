using {f} for uint;
library L {}
function f(uint) {}
contract C { using L for *; }

// ----
