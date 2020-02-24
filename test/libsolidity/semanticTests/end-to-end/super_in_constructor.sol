
		contract A { function f() public virtual returns (uint r) { return 1; } }
		contract B is A { function f() public virtual override returns (uint r) { return super.f() | 2; } }
		contract C is A { function f() public virtual override returns (uint r) { return super.f() | 4; } }
		contract D is B, C { uint data; constructor() public { data = super.f() | 8; } function f() public override (B, C) returns (uint r) { return data; } }
	
// ----
// f() -> 15

