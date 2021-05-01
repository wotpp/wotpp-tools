#include <wpp/frontend/parser/parser.hpp>
#include <wpp/misc/argp.hpp>

void visualize_ast(wpp::node_t node_id, wpp::Env& env, int depth=0) {
	for (int i = 0; i < depth; i++)
		std::cout << " ";

	using namespace wpp;
	wpp::visit(env.ast[node_id],
			   [&] (const IntrinsicRun& x)    {
				   std::cout << "run() [intrinsic]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const IntrinsicPipe& x)   {
				   std::cout << "pipe() [intrinsic]" << std::endl;
				   visualize_ast(x.cmd, env, depth+1);
				   visualize_ast(x.value, env, depth+1);
			   },
			   [&] (const IntrinsicError& x)  {
				   std::cout << "error() [intrinsic]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const IntrinsicLog& x)    {
				   std::cout << "log() [intrinsic]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const IntrinsicAssert& x) {
				   std::cout << "assert() [intrinsic]" << std::endl;
				   visualize_ast(x.lhs, env, depth+1);
				   visualize_ast(x.rhs, env, depth+1);
			   },
			   [&] (const IntrinsicFile& x)   {
				   std::cout << "file() [intrinsic]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const IntrinsicUse& x)    {
				   std::cout << "use() [intrinsic]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const FnInvoke& x) {
				   std::cout << x.identifier.str() << "() [fn invoke]" << std::endl;
				   for (const auto& arg: x.arguments)
					   visualize_ast(arg, env, depth+1);
			   },
			   [&] (const Fn& x) {
				   std::cout << "let " << x.identifier.str() << "(";
				   for (const auto& param: x.parameters)
					   std::cout << param.str() << ", ";
				   std::cout << ") [fn]" << std::endl;
				   visualize_ast(x.body, env, depth+1);
			   },
			   [&] (const Codeify& x)  {
				   std::cout << "[codeify]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const VarRef& x)   {
				   std::cout << x.identifier.str() << " [var ref]" << std::endl;
			   },
			   [&] (const Var& x)      {
				   std::cout << "let " << x.identifier.str() << " [var]" << std::endl;
				   visualize_ast(x.body, env, depth+1);
			   },
			   [&] (const Pop& x)      {
				   std::cout << "pop " << x.identifier.str() << "[pop]" << std::endl;
				   for (const auto& arg: x.arguments)
					   visualize_ast(arg, env, depth+1);
			   },
			   [&] (const New& x)      {
				   std::cout << "[new]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const Drop& x)     {
				   std::cout << "drop " << x.identifier.str() << " " << x.n_args << (x.is_variadic ? "*" : "") << " [drop]" << std::endl;
			   },
			   [&] (const String& x)   {
				   std::cout << "\"" << x.value << "\" [string]" << std::endl;
			   },
			   [&] (const Concat& x)   {
				   std::cout << ".. [concat]" << std::endl;
				   visualize_ast(x.lhs, env, depth+1);
				   visualize_ast(x.rhs, env, depth+1);
			   },
			   [&] (const Slice& x)    {
				   std::cout << "[" << x.start.str() << ":" << x.stop.str() << "] [slice]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
			   },
			   [&] (const Block& x)    {
				   std::cout << "[block]" << std::endl;
				   for (const auto& statement: x.statements)
					   visualize_ast(statement, env, depth+1);
			   },
			   [&] (const Match& x)    {
				   std::cout << "[match]" << std::endl;
				   visualize_ast(x.expr, env, depth+1);
				   for (const auto& [cond, expr]: x.cases) {
					   visualize_ast(cond, env, depth+2);
					   visualize_ast(expr, env, depth+2);
				   }
				   visualize_ast(x.default_case, env, depth+1);
			   },
			   [&] (const Document& x) {
				   for (const auto& statement: x.statements)
					   visualize_ast(statement, env);
			   });
}

int main(int argc, const char *argv[]) {
	std::vector<const char*> positional;

	if (wpp::argparser(
			wpp::Meta{"1.0.0", "A simple utility for visualizing the AST of a Wot++ program."},
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
			if (env.state & wpp::INTERNAL_ERROR_STATE)
				return 1;

			visualize_ast(root, env);
		}

		catch (const wpp::Report& e) {
			std::cerr << e.str();
			return 1;
		}

		catch (const wpp::FileError&) {
			std::cerr << "error: file '" << fname << "' not found\n";
			return 1;
		}

		std::filesystem::current_path(initial_path);
	}
}
