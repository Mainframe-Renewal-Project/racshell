#include "include/racshell/commands/entity_command.hpp"

int main(int argc, char *argv[])
{
	const racshell::EntityCommandSpec spec = {
		.command_name = "resetpassword",
		.operation = "alter",
		.success_verb = "password reset",
		.admin_type = "user",
		.entity_argument = "userid",
		.entity_help = "RACF userid to reset the password for",
		.entity_validation_error = "Invalid input, must be a valid RACF userid",
		.supports_traits = false,
		.traits_help = nullptr,
		.success_label = "User",
		.max_name_length = 8,
		.setup_extra_args = [](argparse::ArgumentParser &program)
		{
			program.add_argument("password")
				.help("new password for the user");
		},
		.apply_extra_args = [](argparse::ArgumentParser &program, nlohmann::json &request)
		{
			request["traits"]["base:password"] = program.get<std::string>("password");
		}};

	return racshell::run_entity_command(argc, argv, spec);
}
