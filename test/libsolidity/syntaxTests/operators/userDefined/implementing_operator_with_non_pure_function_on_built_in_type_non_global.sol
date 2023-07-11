// This should point out all 3 errors, rather than give up after the first one.
using {add as +} for address;

function add(address, address) view returns (address) {}
// ----
// TypeError 3320: (87-90): Operators can only be defined in a global 'using for' directive.
// TypeError 7775: (87-90): Only pure free functions can be used to define operators.
// TypeError 5332: (87-90): Operators can only be implemented for user-defined value types.
