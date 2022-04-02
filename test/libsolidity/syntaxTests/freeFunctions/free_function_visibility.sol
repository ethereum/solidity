function fun1() public { }
function fun2() internal { }
// ----
// SyntaxError 4126: (0-26='function fun1() public { }'): Free functions cannot have visibility.
// SyntaxError 4126: (27-55='function fun2() internal { }'): Free functions cannot have visibility.
