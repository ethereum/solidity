// TODO: What if an error is imported and alias as Panic?
// TODO I Think the best way would be to have Error in the global scope.
error Panic(bytes2);
// ----
// SyntaxError 1855: (131-151): The built-in errors "Error" and "Panic" cannot be re-defined.
