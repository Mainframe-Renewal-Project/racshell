#include "include/racshell/commands/entity_command.hpp"

int main(int argc, char *argv[])
{
	const racshell::EntityCommandSpec spec = {
		.command_name = "resetpassphrase",
		.operation = "alter",
		.success_verb = "passphrase reset",
		.admin_type = "user",
		.entity_argument = "userid",
		.entity_help = "RACF userid to reset the passphrase for",
		.entity_validation_error = "Invalid input, must be a valid RACF userid",
		.supports_traits = false,
		.traits_help = nullptr,
		.success_label = "User",
		.max_name_length = 8,
		.setup_extra_args = [](argparse::ArgumentParser &program)
		{
			program.add_argument("passphrase")
				.help("new passphrase for the user");
		},
		.apply_extra_args = [](argparse::ArgumentParser &program, nlohmann::json &request)
		{
			request["traits"]["base:passphrase"] = program.get<std::string>("passphrase");
		}};
        // TODO: Remove any password that may have been set, like the NOPASSWORD command.

	return racshell::run_entity_command(argc, argv, spec);
}
