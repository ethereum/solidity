{
  returndatacopy(1, 1, 0)
  calldatacopy(1, 1, 0)
  extcodecopy(1, 1, 1, 0)
  codecopy(1, 1, 0)
  log0(1, 0)
  log1(1, 0, 1)
  log2(1, 0, 1, 1)
  log3(1, 0, 1, 1, 1)
  log4(1, 0, 1, 1, 1, 1)
  pop(create(1, 1, 0))
  pop(create2(1, 1, 0, 1))
  pop(call(1, 1, 1, 1, 0, 1, 0))
  pop(callcode(1, 1, 1, 1, 0, 1, 0))
  pop(delegatecall(1, 1, 1, 0, 1, 0))
  pop(staticcall(1, 1, 1, 0, 1, 0))
  return(1, 0)
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
//   RETURNDATACOPY(0, 1, 0)
//   CALLDATACOPY(0, 1, 0)
//   EXTCODECOPY(1, 0, 1, 0)
//   CODECOPY(0, 1, 0)
//   LOG0(0, 0)
//   LOG1(0, 0, 1)
//   LOG2(0, 0, 1, 1)
//   LOG3(0, 0, 1, 1, 1)
//   LOG4(0, 0, 1, 1, 1, 1)
//   CREATE(1, 0, 0)
//   CREATE2(1, 0, 0, 1)
//   CALL(1, 1, 1, 0, 0, 1, 0)
//   CALLCODE(1, 1, 1, 0, 0, 1, 0)
//   DELEGATECALL(1, 1, 0, 0, 1, 0)
//   STATICCALL(1, 1, 0, 0, 1, 0)
//   RETURN(0, 0)
// Memory dump:
// Storage dump:
