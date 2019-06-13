from z3 import *

class Rule:
	def __init__(self):
		self.requirements = []
		self.constraints = []
		self.solver = Solver()
		self.setTimeout(60000)

	def setTimeout(self, _t):
		self.solver.set("timeout", _t)

	def __lshift__(self, _c):
		self.constraints.append(_c)

	def require(self, _r):
		self.requirements.append(_r)

	def check(self, _nonopt, _opt):
		self.solver.add(self.requirements)
		result = self.solver.check()

		if result == unknown:
			raise BaseException('Unable to satisfy requirements.')
		elif result == unsat:
			raise BaseException('Requirements are unsatisfiable.')

		self.solver.push()
		self.solver.add(self.constraints)
		self.solver.add(_nonopt != _opt)

		result = self.solver.check()
		if result == unknown:
			raise BaseException('Unable to prove rule.')
		elif result == sat:
			m = self.solver.model()
			raise BaseException('Rule is incorrect.\nModel: ' + str(m))
		self.solver.pop()
