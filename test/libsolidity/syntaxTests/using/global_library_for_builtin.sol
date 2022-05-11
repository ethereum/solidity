using L for uint global;
using L for uint[] global;
using L for function() returns (uint) global;
library L {
}
// ----
// TypeError 8841: (0-24): Can only use "global" with user-defined types.
// TypeError 8841: (25-51): Can only use "global" with user-defined types.
// TypeError 8841: (52-97): Can only use "global" with user-defined types.
