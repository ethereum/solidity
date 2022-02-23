contract test1 {
  constructor() view {}
}
contract test2 {
  constructor() pure {}
}
// ----
// TypeError 1558: (19-40): Constructor must be payable or non-payable, but is "view".
// TypeError 1558: (62-83): Constructor must be payable or non-payable, but is "pure".
