

![C++ Version](https://img.shields.io/badge/https%3A%2F%2Fimg.shields.io%2Fbadge%2Fany_text-C%2B%2B17-blue?label=C%2B%2B%20Version)
![z/OS Version](https://img.shields.io/badge/https%3A%2F%2Fimg.shields.io%2Fbadge%2Fany_text-z%2FOS%203.1%2B-blue?label=OS)

![RACSHELL Logo](https://github.com/Mainframe-Renewal-Project/racshell/blob/main/logo.png)

# RACSHELL

Open source RACF shell interface intended for z/OS Unix SSH sessions. Built with SEAR and C++17.

## Permissions

To use RACSHELL you will need access to one or more of the RACF profiles documented in the [SEAR documentation](https://mainframe-renewal-project.github.io/sear-docs/authorizations/), depending on the commands you intend to execute. You will also need access to group special, system special, read-only auditor, or FIELD class profiles in addition to the IRRSEQ00, IRRSDL64, and IRRSMO00 API accesses. Whether you can make do with something like group/system special or group/system auditor depends on whether or not you need to change or list things in RACF.

## Commands

The following commands are currently implemented in `racshell/`.

### Command Overview

| Command | Category | Purpose | Key Options |
| --- | --- | --- | --- |
| `adduser` | User | Create RACF user | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `altuser` | User | Alter RACF user | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deleteuser` | User | Delete RACF user | `--json`, `--debug`, `--all-json`, `--no-color` |
| `listuser` | User | Display RACF user details | `--groups`, `--tso`, `--kerberos`, `--cics`, `--omvs`, `--csdata`, `--json`, `--all-json`, `--debug`, `--no-color` |
| `searchusers` | User | Search RACF users | `--filter`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `compareusers` | User | Compare two RACF users | `--json`, `--all-json`, `--debug`, `--no-color` |
| `resume` | User | Resume a RACF user | `--json`, `--debug`, `--all-json`, `--no-color` |
| `revoke` | User | Revoke a RACF user | `--json`, `--debug`, `--all-json`, `--no-color` |
| `resetpassword` | User | Reset RACF user password | `--json`, `--debug`, `--all-json`, `--no-color` |
| `resetpassphrase` | User | Reset RACF user passphrase | `--json`, `--debug`, `--all-json`, `--no-color` |
| `addgroup` | Group | Create RACF group | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `altgroup` | Group | Alter RACF group | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deletegroup` | Group | Delete RACF group | `--json`, `--debug`, `--all-json`, `--no-color` |
| `connect` | Group | Connect user to RACF group | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `remove` | Group | Remove user from RACF group | `--json`, `--debug`, `--all-json`, `--no-color` |
| `listgroup` | Group | Display RACF group details | `--users`, `--omvs`, `--csdata`, `--json`, `--all-json`, `--debug`, `--no-color` |
| `searchgroups` | Group | Search RACF groups | `--filter`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `comparegroups` | Group | Compare two RACF groups | `--users`, `--omvs`, `--csdata`, `--json`, `--all-json`, `--debug`, `--no-color` |
| `groupchart` | Group | Draw the group hierarchy as an org chart | `--levels`, `--width`, `--vertical`, `--omvs`, `--users`, `--json`, `--debug`, `--no-color` |
| `adddataset` | Dataset | Create RACF dataset profile | `--generic`, `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `altdataset` | Dataset | Alter RACF dataset profile | `--generic`, `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deletedataset` | Dataset | Delete RACF dataset profile | `--generic`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `listdataset` | Dataset | Display RACF dataset profile details | `--generic`, `--csdata`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `searchdatasets` | Dataset | Search RACF dataset profiles | `--filter`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `addresource` | Resource | Create RACF resource profile | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `altresource` | Resource | Alter RACF resource profile | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deleteresource` | Resource | Delete RACF resource profile | `--json`, `--debug`, `--all-json`, `--no-color` |
| `listresource` | Resource | Display RACF resource profile details | `--csdata`, `--access`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `searchresource` | Resource | Search RACF resource profiles by class | `--filter`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `permitdataset` | Permit | Grant access to dataset profile | `--generic`, `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `permitresource` | Permit | Grant access to resource profile | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deletepermitdataset` | Permit | Revoke access to dataset profile | `--generic`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deletepermitresource` | Permit | Revoke access to resource profile | `--json`, `--debug`, `--all-json`, `--no-color` |
| `addkeyring` | Keyring | Create a RACF keyring | `--json`, `--debug`, `--all-json`, `--no-color` |
| `deletekeyring` | Keyring | Delete a RACF keyring | `--json`, `--debug`, `--all-json`, `--no-color` |
| `listkeyring` | Keyring | Display keyring and certificate details | `--json`, `--debug`, `--all-json`, `--no-color` |
| `racfctl` | System | Display or alter RACF system options (SETROPTS) | `list`, `modify`, `refresh`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `rrsflist` | RRSF | Display RRSF configuration and nodes (read-only) | `--nodes`, `--settings`, `--json`, `--debug`, `--all-json`, `--no-color` |

All commands accept `-n`, `--no-color`, or `--no-colour` to disable colored output.

## System Options

### racfctl

Display or alter RACF system options from z/OS Unix, using SEAR's `racf-options` interface rather than invoking TSO SETROPTS.

Usage:

`racfctl list [options]`

`racfctl modify [<trait> ...] [options]`

`racfctl refresh <class> [--generic] [options]`

`list` displays system settings in grouped, colored sections. Class lists are hidden by default. `modify` requires at least one system option flag or positional `key=value` trait and performs an alter request. Flags and positional traits can be combined, but the same trait cannot be supplied in both forms. Positional values are parsed as booleans, integers, or strings; flags preserve the data types expected by SEAR.

List detail flags (`list` only, can be combined):

- `-c, --classes` display all class lists
- `--active-classes` display active classes
- `--raclist` display RACLIST classes
- `--generic` display generic profile checking classes
- `--genlist` display generic profile sharing classes
- `--generic-command-classes` display generic command classes
- `--audit-classes` display profile change auditing classes
- `--statistics-classes` display statistics classes
- `--global-access-classes` display global access classes
- `--logging-classes` display audit logging class lists

These selectors also apply to `--json`. Use `--all-json` for the unfiltered SEAR response, including all class lists.

`refresh` requires one class name and refreshes RACLIST profiles by default. `-g, --generic` selects GENERIC profiles instead. Class names are normalized to uppercase. A bare `racfctl refresh` is rejected; there is no automatic all-class refresh. Refreshing is a separate subcommand, not a `modify --refresh` flag.

Common system option flags (`modify` only):

| Flag | Value | Purpose |
| --- | --- | --- |
| `--classact`, `--noclassact` | classes | Activate or deactivate classes |
| `--raclist`, `--noraclist` | classes | Enable or disable RACLIST processing |
| `--generic` | classes | Enable generic profile checking |
| `--genlist`, `--nogenlist` | classes | Enable or disable generic profile sharing |
| `--audit`, `--noaudit` | classes | Enable or disable profile change auditing |
| `--password-interval` | interval | Maximum password change interval |
| `--min-password-interval` | interval | Minimum password change interval |
| `--passphrase-interval` | interval | Passphrase change interval |
| `--password-warning` | interval | Password expiration warning interval |
| `--inactive-interval` | interval | Inactive userid revocation interval |
| `--password-history` | unsigned integer | Previous passwords to remember |
| `--max-password-attempts` | unsigned integer | Incorrect password attempts before revocation |
| `--mixed-case-passwords` | `true` or `false` | Mixed-case password support |
| `--special-character-passwords` | `true` or `false` | Special password character support |
| `--program-control` | `true` or `false` | Program control |
| `--special-user-audit` | `true` or `false` | Audit commands issued by SPECIAL users |
| `--operator-audit` | `true` or `false` | Audit operator actions |
| `--group-list` | `true` or `false` | List-of-groups access checking |
| `--initstat` | `true` or `false` | User verification statistics |

Quote space-separated class names when specifying multiple classes, for example `--classact 'FACILITY XFACILIT'`. Interval flags are passed as SEAR strings; integer flags reject negative or nonnumeric values.

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output the extracted profile or operation status as JSON
- `-a, --all-json` output full raw SEAR JSON response; failed requests still return a nonzero exit status

Examples:

```sh
racfctl list
racfctl list --active-classes
racfctl list --raclist --generic
racfctl list --classes
racfctl list --json
racfctl modify --max-password-attempts 5 --password-history 10
racfctl refresh appl
racfctl refresh --generic DATASET
racfctl modify --mixed-case-passwords true --special-user-audit false
racfctl modify --classact 'FACILITY XFACILIT' base:program_control=true
```

Warning: changes affect the whole system. Review planned changes before executing them, and protect extracted settings and debug output as sensitive information.

See SEAR's [system options examples](https://mainframe-renewal-project.github.io/sear-docs/python/setropts/) and [supported RACF options traits](https://mainframe-renewal-project.github.io/sear-docs/traits/racf_options/) for the full set of available settings, including RACLIST refresh traits.

## RRSF

### rrsflist

Display the RRSF configuration using SEAR's `racf-rrsf` extract interface. This command is read-only; SEAR does not support adding, modifying, deleting, or searching RRSF configuration.

Usage:

`rrsflist [options]`

By default, the command shows subsystem and direction settings with a compact overview of each node. Detailed workspace, network, and notification information is optional.

Options:

- `-N, --nodes` display full node details
- `-s, --settings` display notification destinations and output settings
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output the selected configuration details as JSON
- `-a, --all-json` output the unfiltered raw SEAR JSON response; failed requests still return a nonzero exit status

Examples:

```sh
rrsflist
rrsflist --nodes --settings
rrsflist --json
rrsflist --all-json
```

RRSF data and debug output contain sensitive system-layout and network information. Store them securely. See SEAR's [RRSF documentation](https://mainframe-renewal-project.github.io/sear-docs/python/rrsf/) for details and required API support.

## User Commands

User administration and lookup commands.

### adduser

Create a RACF user.

Usage:

`adduser <userid> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:name='John Doe' omvs:uid=24`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### altuser

Alter an existing RACF user.

Usage:

`altuser <userid> [options]`

Options:

- `-t, --traits` traits to alter (repeatable), e.g. `base:name='LEONARD CARCARAMO' base:special=true`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### deleteuser

Delete a RACF user.

Usage:

`deleteuser <userid> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### listuser

Extract and display RACF user details.

Usage:

`listuser <user> [options]`

Options:

- `-g, --groups` list connected RACF groups
- `-t, --tso` list TSO segment
- `-k, --kerberos` list Kerberos segment
- `-c, --cics` list CICS segment
- `-o, --omvs` list OMVS segment
- `-x, --csdata` list CSDATA segment
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### searchusers

Search for RACF users.

Usage:

`searchusers [options]`

Options:

- `-f, --filter` filter users by userid pattern (optional), e.g. `IBM`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### compareusers

Compare two RACF users and report any differences in their extracted data.

Usage:

`compareusers <left-user> <right-user> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output differences as JSON
- `-a, --all-json` output full raw SEAR JSON responses

### resume

Resume a RACF user.

Usage:

`resume <userid> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### revoke

Revoke a RACF user.

Usage:

`revoke <userid> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### resetpassword

Reset a RACF user's password.

Usage:

`resetpassword <userid> <password> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### resetpassphrase

Reset a RACF user's passphrase.

Usage:

`resetpassphrase <userid> <passphrase> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

NOTE: this is not tested, and is lacking of NOPASSWORD to ensure the password is not kept

### Group Commands

Group administration and lookup commands.

### addgroup

Create a RACF group.

Usage:

`addgroup <group> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:owner=SYS1 base:superior_group=SYS1`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### altgroup

Alter an existing RACF group.

Usage:

`altgroup <group> [options]`

Options:

- `-t, --traits` traits to alter (repeatable), e.g. `base:owner=SYS1 omvs:gid=1234567`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### deletegroup

Delete a RACF group.

Usage:

`deletegroup <group> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### listgroup

Extract and display RACF group details.

Usage:

`listgroup <group> [options]`

Options:

- `-u, --users` list connected users
- `-o, --omvs` list OMVS segment
- `-x, --csdata` list CSDATA segment
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### searchgroups

Search for RACF groups.

Usage:

`searchgroups [options]`

Options:

- `-f, --filter` filter groups by name pattern (optional), e.g. `SYS`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### comparegroups

Compare two RACF groups and report any differences in their extracted data.

Usage:

`comparegroups <left-group> <right-group> [options]`

Options:

- `-u, --users` compare connected users
- `-o, --omvs` compare OMVS segment
- `-x, --csdata` compare CSDATA segment
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output differences as JSON
- `-a, --all-json` output full raw SEAR JSON responses

### groupchart

Draw a RACF group hierarchy. Omit the root to show all top-level groups.

Usage:

`groupchart [root] [options]`

Options:

- `-l, --levels` maximum levels below the root (0 for unlimited)
- `-w, --width` maximum chart width before switching to vertical layout (0 for terminal width)
- `-v, --vertical` stack subgroups vertically
- `-o, --omvs` show OMVS segment presence and GID for each group
- `-u, --users` show connected user count for each group, capped at `100+`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON

With `--omvs` and `--users`, each requested detail appears on its own line below the group name. The OMVS line shows `no OMVS` or `OMVS GID=<gid>` (`?` if the GID is unavailable); the user line shows `users=N` or `users=100+`. JSON nodes include an `omvs` boolean and a `gid` number or `null` when `--omvs` is enabled, and `user_count` as a number or `100+` when `--users` is enabled.

### connect

Connect a RACF user to a RACF group.

Usage:

`connect <userid> <group> [options]`

Options:

- `-t, --traits` connection traits to set (repeatable), e.g. `base:special=true`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### remove

Remove a RACF user from a RACF group.

Usage:

`remove <userid> <group> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

## Dataset Commands

Dataset profile administration commands.

### adddataset

Create a RACF dataset profile.

Usage:

`adddataset <dataset> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:universal_access=None base:owner=eswift`
- `-g, --generic` treat the dataset name as a generic profile (e.g. `ESWIFT.TEST.**`)
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### altdataset

Create a RACF dataset profile.

Usage:

`altdataset <dataset> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:universal_access=None base:owner=eswift`
- `-g, --generic` treat the dataset name as a generic profile (e.g. `ESWIFT.TEST.**`)
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### deletedataset

Delete a RACF dataset profile.

Usage:

`deletedataset <dataset> [options]`

Options:

- `-g, --generic` treat the dataset name as a generic profile
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### listdataset

Extract and display RACF dataset profile details.

Usage:

`listdataset <dataset> [options]`

Options:

- `-g, --generic` treat the dataset name as a generic profile
- `-x, --csdata` list CSDATA segment
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### searchdatasets

Search for RACF dataset profiles.

Usage:

`searchdatasets [options]`

Options:

- `-f, --filter` filter dataset profiles by name pattern (optional), e.g. `LEONARD`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

## Resource Commands

Resource profile administration and lookup commands.

### addresource

Create a RACF resource profile.

Usage:

`addresource <resource> <class> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:owner=SECADM base:universal_access=None`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### altresource

Alter an existing RACF resource profile.

Usage:

`altresource <resource> <class> [options]`

Options:

- `-t, --traits` traits to alter (repeatable), e.g. `base:owner=SECADM base:universal_access=None`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### deleteresource

Delete a RACF resource profile.

Usage:

`deleteresource <resource> <class> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### listresource

Extract and display RACF resource profile details.

Usage:

`listresource <resource> <class> [options]`

Options:

- `-x, --csdata` list CSDATA segment
- `-l, --access` list access control list
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### searchresource

Search for RACF resource profiles in a given class.

Usage:

`searchresource <class> [options]`

Options:

- `-f, --filter` filter resource profiles by name pattern (optional), e.g. `IRR`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

## Permit Commands

Permit (access control list) administration and management commands.

### permitdataset

Grant a user access to a RACF dataset profile.

Usage:

`permitdataset <dataset> [--volume <volume>] <userid> [options]`

Options:

- `-g, --generic` treat the dataset name as a generic profile (e.g. `ESWIFT.TEST.**`)
- `--volume` volume name for the dataset
- `-t, --traits` permit traits (repeatable), e.g. `base:access=READ`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### permitresource

Grant a user access to a RACF resource profile.

Usage:

`permitresource <resource> <class> <userid> [options]`

Options:

- `-t, --traits` permit traits (repeatable), e.g. `base:access=READ`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### deletepermitdataset

Revoke a user's access to a RACF dataset profile.

Usage:

`deletepermitdataset <dataset> [--volume <volume>] <userid> [options]`

Options:

- `-g, --generic` treat the dataset name as a generic profile
- `--volume` volume name for the dataset
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### deletepermitresource

Revoke a user's access to a RACF resource profile.

Usage:

`deletepermitresource <resource> <class> <userid> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

## Keyring Commands

Keyring administration and lookup commands. Each command requires both the keyring name and owner userid.

### addkeyring

Create a RACF keyring.

Usage:

`addkeyring <owner> <keyring> [options]`

### deletekeyring

Delete a RACF keyring.

Usage:

`deletekeyring <owner> <keyring> [options]`

### listkeyring

Extract keyring details, including associated certificates.

Usage:

`listkeyring <owner> <keyring> [options]`

All keyring commands support `-n, --no-color`, `-d, --debug`, `-j, --json`, and `-a, --all-json`.

## Build pre-reqs

To compile RACSHELL you will need the following software:

- z/OS 3.1 or later
- Git from Open Enterprise Foundation
- CMake from zOpen
- zoslib from zOpen
- OpenSSL 4.x.x or higher from zOpen
- IBM Clangd compiler 2.1 or higher (C++17 is a hard pre-requisite)

First pull in the RACSHELL repository:

```sh
git clone https://github.com/Mainframe-Renewal-Project/racshell.git
```

Make sure to pull in the SEAR submodule before compile, git doesn't do this automatically:

```sh
git submodule update --init --recursive sear
```

Then kick off the compilation process:

```sh
cmake --preset zos -DZOSLIB:FILEPATH="$ZOSLIB_ROOT/lib/libzoslib.a"
cmake --build --preset zos --target racshell_commands
```

After compilation the commands can be found in the build output directory specified by the CMake preset (./build/zos/).
