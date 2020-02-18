
		contract A { function f() public { super; } }
	
// ====
// optimize-yul: false
// ----
// f() -> 

