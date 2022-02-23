{
	for { function f() {} } 1:bool {} {}
}
// ----
// SyntaxError 3441: (9-17): Functions cannot be defined inside a for-loop init block.
