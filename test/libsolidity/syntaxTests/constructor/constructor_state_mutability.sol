contract test1 {
  constructor() public view {}
}
contract test2 {
  constructor() public pure {}
}
// ----
// TypeError 1558: (19-47): Constructor must be payable or non-payable, but is "view".
// TypeError 1558: (69-97): Constructor must be payable or non-payable, but is "pure".
