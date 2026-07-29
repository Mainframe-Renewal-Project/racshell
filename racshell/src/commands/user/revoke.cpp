#include "include/racshell/commands/entity_command.hpp"

int main(int argc, char *argv[])
{
	const racshell::EntityCommandSpec spec = {
		.command_name = "revoke",
		.operation = "alter",
		.success_verb = "revoked",
		.admin_type = "user",
		.entity_argument = "userid",
		.entity_help = "RACF userid to revoke",
		.entity_validation_error = "Invalid input, must be a valid RACF userid",
		.supports_traits = false,
		.traits_help = nullptr,
		.success_label = "User",
		.max_name_length = 8,
		.apply_extra_args = [](argparse::ArgumentParser &, nlohmann::json &request)
		{
			request["traits"]["base:revoke_date"] = "";
		}};

	return racshell::run_entity_command(argc, argv, spec);
}
