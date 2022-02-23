{
	for { for {function foo() {}} 1:bool {} {} } 1:bool {} {}
}
// ----
// SyntaxError 3441: (14-22): Functions cannot be defined inside a for-loop init block.
