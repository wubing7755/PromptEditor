#ifndef PP_CLI_H
#define PP_CLI_H

/**
 * Entry point for the pp CLI.  Dispatches argv[1] to the appropriate
 * command handler.
 *
 * @return 0 on success, 1 on error, 2 for planned-but-unimplemented commands.
 */
int pp_cli_run(int argc, char **argv);

#endif
