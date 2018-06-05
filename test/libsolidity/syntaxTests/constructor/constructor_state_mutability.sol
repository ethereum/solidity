contract test1 {
  constructor() constant {}
}
contract test2 {
  constructor() view {}
}
contract test3 {
  constructor() pure {}
}
// ----
// TypeError: (19-44): Constructor must be payable or non-payable, but is "view".
// TypeError: (66-87): Constructor must be payable or non-payable, but is "view".
// TypeError: (109-130): Constructor must be payable or non-payable, but is "pure".
