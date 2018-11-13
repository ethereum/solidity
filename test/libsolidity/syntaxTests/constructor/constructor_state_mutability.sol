contract test1 {
  constructor() public view {}
}
contract test2 {
  constructor() public pure {}
}
// ----
// TypeError: (19-47): Constructor must be payable or non-payable, but is "view".
// TypeError: (69-97): Constructor must be payable or non-payable, but is "pure".
