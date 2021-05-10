#include <wpp/frontend/parser/parser.hpp>
#include <wpp/misc/argp.hpp>

inline std::string escape(std::string src) {
	std::string result;
	for (auto it = src.begin(); it != src.end(); it++) {
		switch (*it) {
		case '\n':
			if (src.size() < 80)
				result += "\\n";
			else
				result += "\n";

			break;
		case '\t':
			if (it != result.begin())
				if (*(it-1) == '\n')
					result += "\t";
				else
					result += "\\t";
			else
				result += "\\t";

			break;
		case '\"':
			result += "\\\"";
			break;
		case '\'':
			result += "\\\'";
			break;
		default:
			result += *it;
			break;
		}
	}

	return result;
}

template <typename T, typename S>
constexpr std::vector<S> apply(const std::vector<T>& list, auto&& Pred) {
	std::vector<S> result;
	for (const auto& t: list)
		result.push_back(Pred(t));

	return result;
}

inline std::string tabs(int count) {
	std::string result;
	for (int i = 0; i < count; i++)
		result += "\t";

	return result;
}

inline std::string params(const std::vector<std::string>& args) {
	std::string result = "";
	size_t i = 0;
	for (const auto& arg: args) {
		result += arg;
		if (i++ < args.size() - 1)
			result += ", ";
	}

	return result;
}

inline std::string call(std::string func, const std::vector<std::string>& args) {
	return func + "(" + params(args) + ")";
}

inline std::string intrinsic(const std::string& kw, const auto&&... args) {
	return kw + " " + (std::string(args) + " " + ...);
}

inline std::string format(wpp::node_t node_id, wpp::Env& env, int tab_width=0) {
	using namespace wpp;
	return wpp::visit(env.ast[node_id],
					  [&] (const IntrinsicRun& x) {
						  return intrinsic("run", format(x.expr, env, tab_width));
					  },
					  [&] (const IntrinsicPipe& x) {
						  return intrinsic("pipe", format(x.cmd, env, tab_width), format(x.value, env, tab_width));
					  },
					  [&] (const IntrinsicError& x) {
						  return intrinsic("error", format(x.expr, env, tab_width));
					  },
					  [&] (const IntrinsicLog& x) {
						  return intrinsic("log", format(x.expr, env, tab_width));
					  },
					  [&] (const IntrinsicAssert& x) {
						  return intrinsic("assert", format(x.lhs, env, tab_width), format(x.rhs, env, tab_width));
					  },
					  [&] (const IntrinsicFile& x) {
						  return intrinsic("file", format(x.expr, env, tab_width));
					  },
					  [&] (const IntrinsicUse& x) {
						  return intrinsic("use", format(x.expr, env, tab_width));
					  },
					  [&] (const FnInvoke& x) {
						  return call(x.identifier.str(), apply<node_t, std::string>(x.arguments, [&env, &tab_width](node_t node_id) { return format(node_id, env, tab_width); }));
					  },
					  [&] (const Fn& x) {
						  auto let = "let " + call(x.identifier.str(), apply<wpp::View, std::string>(x.parameters, [](wpp::View view) { return view.str(); }));
						  auto body = format(x.body, env, tab_width);

						  if (std::holds_alternative<Block>(env.ast[x.body]))
							let += " " + body;
						  else {
    						  	if (body.size() > 16)
								let += "\n" + tabs(tab_width+1) + format(x.body, env, tab_width+1);
    						  	else
        						  	let += " " + body;
						  }

						  return let + "\n";
					  },
					  [&] (const Codeify& x) {
						  return "!" + format(x.expr, env, tab_width);
					  },
					  [&] (const VarRef& x) {
						  return x.identifier.str();
					  },
					  [&] (const Var& x) {
						  auto let = "let " + x.identifier.str();
						  auto body = format(x.body, env, tab_width);

						  if (std::holds_alternative<Block>(env.ast[x.body]))
							let += " " + body;
						  else {
    						  	if (body.size() > 16)
								let += "\n" + tabs(tab_width+1) + format(x.body, env, tab_width+1);
    						  	else
        						  	let += " " + body;
						  }

						  return let + "\n";
					  },
					  [&] (const Pop& x) {
						  // std::cerr << x.identifier << ": " << x.n_popped_args << std::endl;
						  auto args = apply<node_t, std::string>(x.arguments, [&env, &tab_width](node_t node_id) { return format(node_id, env, tab_width); });
						  for (size_t i = 0; i < x.n_popped_args; i++)
							  args.push_back("*");

						  return "pop " + call(x.identifier.str(), args);
					  },
					  [&] (const New& x) {
						  return "new " + format(x.expr, env, tab_width);
					  },
					  [&] (const Drop& x) {
						  std::string result = "drop " + x.identifier.str() + "(";
						  for (size_t i = 0; i < x.n_args; i++)
							  result += ". ";
						  if (x.is_variadic)
							  result += "*";

						  result += ")";
						  return result;
					  },
					  [&] (const String& x) {
						  return "\"" + escape(x.value) + "\"";
					  },
					  [&] (const Concat& x) {
						  return format(x.lhs, env, tab_width) + " .. " + format(x.rhs, env, tab_width);
					  },
					  [&] (const Slice& x) {
						  return format(x.expr, env, tab_width) + "[" + x.start.str() + ":" + x.stop.str() + "]";
					  },
					  [&] (const Block& x) {
						  std::string result = "{\n";
						  for (const auto& statement: x.statements)
							  result += tabs(tab_width+1) + format(statement, env, tab_width+1) + "\n";
						  result += tabs(tab_width+1) + format(x.expr, env, tab_width+1) + "\n";
						  result += tabs(tab_width) + "}";

						  return result;
					  },
					  [&] (const Match& x) {
						  std::string result = "match " + format(x.expr, env, tab_width) + " {\n";
						  for (const auto& [cond, expr]: x.cases)
							  result += tabs(tab_width+1) + format(cond, env, tab_width+1) + " -> " + format(expr, env, tab_width+1) + "\n";
						  result += tabs(tab_width+1) + "* -> " + format(x.default_case, env, tab_width+1) + "\n";
						  result += tabs(tab_width) + "}";

						  return result;
					  },
					  [&] (const Document& x) {
						  std::string result = "";
						  for (const auto& statement: x.statements)
							  result += format(statement, env, tab_width) + "\n";

						  return result;
					  });
}

int main(int argc, const char *argv[]) {
	std::vector<const char*> positional;

	if (wpp::argparser(
			wpp::Info{"1.0.0", "A simple utility for visualizing the AST of a Wot++ program."},
			argc, argv, &positional
			))
		return 0;

	const auto initial_path = std::filesystem::current_path();
	for (const auto& fname: positional) {
		// Set current path to path of file.
		const auto path = initial_path / std::filesystem::path{fname};
		std::filesystem::current_path(path.parent_path());

		wpp::Env env{ initial_path, {}, 0 };

		try {
			env.sources.push(path, wpp::read_file(path), wpp::modes::normal);

			wpp::node_t root = wpp::parse(env);
			if (env.state & wpp::ABORT_EVALUATION)
				return 1;

			std::cout << format(root, env) << std::endl;
		}

		catch (const wpp::Report& e) {
			std::cerr << e.str();
			return 1;
		}

		catch (const wpp::FileNotFoundError&) {
				std::cerr << "error: file '" << fname << "' not found\n";
				return 1;
		}

		catch (const wpp::NotFileError&) {
				std::cerr << "error: '" << fname << "' is not a file\n";
				return 1;
		}

		catch (const wpp::FileReadError&) {
				std::cerr << "error: cannot read '" << fname << "'\n";
				return 1;
		}

		catch (const wpp::SymlinkError&) {
				std::cerr << "error: symlink '" << fname << "' resolves to itself\n";
				return 1;
		}


		std::filesystem::current_path(initial_path);
	}
}
