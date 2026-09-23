#ifndef DPICP_STACKMACHINE
#define DPICP_STACKMACHINE

#include <vector>

namespace stack_machine_generator {

	template<class Operation>
	class StackMachine {
	public:
		using u32 = unsigned __int32;

		typedef struct _Element {
			Operation type;
			u32 value;
		} Element;

		// function(ret, opType, args): computes ret from args.size() operands, returns false on failure (e.g. div by zero)
		typedef bool(*StackMachineFunction)(u32&, Operation, const std::vector<u32>&);

		// src: postfix (RPN) element list. Operation::CONSTANT elements are leaves pushed as-is;
		// any other element pops `value` operands off the run stack, calls `function`, and pushes the result as CONSTANT.
		bool run(const std::vector<Element>& src, StackMachineFunction function, u32& result) const {
			std::vector<Element> runstack;

			for (auto it = src.cbegin(); it != src.cend(); ++it) {
				if (it->type == Operation::CONSTANT) {
					runstack.push_back(*it);
					continue;
				}

				if (runstack.size() < it->value)
					return false;

				std::vector<u32> args;
				for (size_t i = runstack.size() - it->value; i < runstack.size(); i++)
					args.push_back(runstack[i].value);
				runstack.resize(runstack.size() - it->value);

				Element ret = {};
				ret.type = Operation::CONSTANT;
				if (!function(ret.value, it->type, args))
					return false;

				runstack.push_back(ret);
			}

			if (runstack.size() != 1)
				return false;

			result = runstack.back().value;
			return true;
		}
	};
}

#endif
