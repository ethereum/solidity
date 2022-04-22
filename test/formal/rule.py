import sys

from typing import Any
from z3 import sat, Solver, unknown, unsat

class Rule:
	def __init__(self) -> None:
		self.requirements = [] # type: Any
		self.constraints = [] # type: Any
		self.solver = Solver()
		self.setTimeout(60000)

	def setTimeout(self, _t: Any) -> None:
		self.solver.set("timeout", _t)

	def __lshift__(self, _c: Any) -> None:
		self.constraints.append(_c)

	def require(self, _r: Any) -> None:
		self.requirements.append(_r)

	def check(self, _nonopt: Any, _opt: Any) -> None:
		self.solver.add(self.requirements)
		result = self.solver.check()

		if result == unknown:
			self.error('Unable to satisfy requirements.')
		elif result == unsat:
			self.error('Requirements are unsatisfiable.')

		self.solver.push()
		self.solver.add(self.constraints)
		self.solver.add(_nonopt != _opt)

		result = self.solver.check()
		if result == unknown:
			self.error('Unable to prove rule.')
		elif result == sat:
			m = self.solver.model()
			self.error('Rule is incorrect.\nModel: ' + str(m))
		self.solver.pop()

	@classmethod
	def error(cls, msg: Any) -> None:
		print(msg)
		sys.exit(1)
