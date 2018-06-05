// It is fine to "override" constructor of a base class since it is invisible
contract A { constructor() public { } }
contract B is A { constructor() public { } }
// ----
// Warning: (135-178): This declaration shadows an existing declaration.
