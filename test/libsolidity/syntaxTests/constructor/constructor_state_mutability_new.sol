contract test1 {
  constructor() public constant {}
}
contract test2 {
  constructor() public view {}
}
contract test3 {
  constructor() public pure {}
}
// ----
// TypeError: (19-51): Constructor must be payable or non-payable, but is "view".
// TypeError: (73-101): Constructor must be payable or non-payable, but is "view".
// TypeError: (123-151): Constructor must be payable or non-payable, but is "pure".
