# RACSHELL

Open source RACF shell interface intended for z/OS Unix SSH sessions

## Commands

The following commands are currently implemented in `racshell/`.

### Command Overview

| Command | Category | Purpose | Key Options |
| --- | --- | --- | --- |
| `adduser` | User | Create RACF user | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `altuser` | User | Alter RACF user | `--traits`, `--json`, `--debug`, `--all-json`, `--no-color` |
| `deleteuser` | User | Delete RACF user | `--json`, `--debug`, `--all-json`, `--no-color` |
| `listuser` | User | Display RACF user details | `--groups`, `--tso`, `--kerberos`, `--cics`, `--omvs`, `--security`, `--csdata`, `--json`, `--all-json`, `--debug`, `--no-color` |
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
- `-s, --security` list security-related fields
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
